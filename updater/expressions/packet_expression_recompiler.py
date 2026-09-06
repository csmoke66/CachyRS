from dataclasses import dataclass
from enum import Enum
from typing import Dict, List

import angr
from angr import *

import claripy

from unicorn import *
from unicorn.x86_const import *

from mlir.dialects import *

import llvmlite.binding as llvm
from llvmlite import ir

class ExpirOpcode(Enum):
    ADD = 1
    SUB = 2
    INVERT = 3
    XOR = 4
    EXTRACT = 5
    CONCAT = 6
    LOAD = 7
    STORE = 8
    SIGN_EXT = 9
    LOAD_CONST = 10
    MUL = 11
    FP_TO_SBV = 12,
    FP_TO_FP = 13,
    INVALID = 14

class ExpirArgType(Enum):
    CONSTANT = 1
    VAR = 2
    FUNC_ARGS = 3

@dataclass
class ExpirArg:
    type: ExpirArgType
    value: any

    def __eq__(self, other):
        if not isinstance(other, ExpirArg):
            return False
        
        return self.type == other.type and self.value == other.value
    
    def __hash__(self):
        return hash((self.type, self.value))

@dataclass
class ExpirNode:
    opcode: ExpirOpcode
    args: list

    def __eq__(self, other):
        if not isinstance(other, ExpirNode):
            return False
        
        return self.opcode == other.opcode and self.args == other.args
    
    def __hash__(self):
        return hash((self.opcode, tuple(self.args)))

@dataclass
class DataAccess:
    hash: int
    symbolic: bool
    expr: any

@dataclass
class IrWalkResult:
    idx: int
    type: ExpirArgType

    def __eq__(self, other):
        if not isinstance(other, IrWalkResult):
            return False
        
        return self.idx == other.idx and self.type == other.type
    
    def __hash__(self):
        return hash((self.idx, self.type))

def compile_expr_var_ref(val_str, var_idx):
    map = {
        "menu_action_0#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 0)
        },
        "menu_action_1#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 1)
        },
        "menu_action_2#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 2)
        },
        "menu_action_3#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 3)
        },
        "packet_buffer#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 4)
        },
        "engine_194a8_490#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 5)
        },
        "FP_unknown_1#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 6)
        },
        "FP_unknown_2#": {
            "arg": ExpirArg(ExpirArgType.FUNC_ARGS, 7)
        },
    }

    for key, value in map.items():
        if val_str.startswith(key):
            return ExpirNode(ExpirOpcode.LOAD, [
                ExpirArg(ExpirArgType.VAR, var_idx),
                ExpirArg(value["arg"].type, value["arg"].value)
            ])

    return None
    
def compile_expr_to_ir(memory_load_registry, memory_store_registry, expr):
    expr_to_id = {}
    ir_unsorted = []
    var_counter = 0

    def walk(node):
        nonlocal var_counter
        
        if node.op == 'BVV':
            return IrWalkResult(node.args[0], ExpirArgType.CONSTANT)
        elif node.op == 'BVS' or node.op == 'FPS':
            val_str = node.args[0]
            node_hash = node.hash()
            print(f"check {node_hash} {node}")

            var_ref = compile_expr_var_ref(val_str, var_counter)
            if var_ref != None:
                last_var_idx = var_counter
                var_counter += 1

                ir_unsorted.append(var_ref)
                return IrWalkResult(last_var_idx, ExpirArgType.VAR)
            
            if node_hash in memory_load_registry:
                access = memory_load_registry[node_hash]
                addr_ast = access.expr
                
                clean_addr_var = walk(addr_ast)
                
                var_idx = var_counter
                var_counter += 1

                ir_unsorted.append(ExpirNode(ExpirOpcode.LOAD, [
                    ExpirArg(ExpirArgType.VAR, var_idx),
                    ExpirArg(clean_addr_var.type, clean_addr_var.idx)
                ]))

                return IrWalkResult(var_idx, ExpirArgType.VAR)

            if node_hash in memory_store_registry:
                access = memory_store_registry[node_hash]
                addr_ast = access.expr
                
                clean_addr_var = walk(addr_ast)
                
                var_idx = var_counter
                var_counter += 1

                ir_unsorted.append(ExpirNode(ExpirOpcode.STORE, [
                    ExpirArg(ExpirArgType.VAR, var_idx),
                    ExpirArg(clean_addr_var.type, clean_addr_var.idx)
                ]))

                return IrWalkResult(var_idx, ExpirArgType.VAR)

            raise ValueError(f"unhandled str {val_str}")
        elif node.hash() in expr_to_id:
            # optimization to reduce duplicate mappings in expressions
            # into a single variable
            return IrWalkResult(expr_to_id[node.hash()], ExpirArgType.VAR)
        elif node.op == 'Concat':
            flattened_args = []
            queue = [node]
            while queue:
                curr = queue.pop(0)
                if curr.op == 'Concat':
                    for arg in reversed(curr.args):
                        queue.insert(0, arg)
                else:
                    flattened_args.append(curr)

            child_vars = [walk(arg) for arg in flattened_args]

            var_idx = var_counter
            var_counter += 1

            opcode = ExpirOpcode.INVALID
            args = []

            args.append(ExpirArg(ExpirArgType.VAR, var_idx))
            if len(set(child_vars)) == 1:
                opcode = ExpirOpcode.SIGN_EXT
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
            elif len(child_vars) > 2 and len(set(child_vars[:-1])) == 1:
                opcode = ExpirOpcode.SIGN_EXT
                args.append(ExpirArg(child_vars[-1].type, child_vars[-1].idx))
            else:
                opcode = ExpirOpcode.CONCAT
                for child in child_vars:
                    args.append(ExpirArg(child.type, child.idx))

            ir_unsorted.append(ExpirNode(opcode, args))

            expr_to_id[node.hash()] = var_idx

            return IrWalkResult(var_idx, ExpirArgType.VAR)
        else:
            child_vars = [walk(arg) for arg in node.args if isinstance(arg, (claripy.ast.bv.BV, claripy.ast.fp.FP))]

            var_idx = var_counter
            var_counter += 1

            opcode = ExpirOpcode.INVALID
            args = []
            
            args.append(ExpirArg(ExpirArgType.VAR, var_idx))
            if node.op == '__invert__':
                opcode = ExpirOpcode.INVERT
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
            elif node.op == '__add__':
                opcode = ExpirOpcode.ADD
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
                args.append(ExpirArg(child_vars[1].type, child_vars[1].idx))
            elif node.op == '__sub__':
                opcode = ExpirOpcode.SUB
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
                args.append(ExpirArg(child_vars[1].type, child_vars[1].idx))
            elif node.op == '__xor__':
                opcode = ExpirOpcode.XOR
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
                args.append(ExpirArg(child_vars[1].type, child_vars[1].idx))
            elif node.op == '__mul__':
                opcode = ExpirOpcode.MUL
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
                args.append(ExpirArg(child_vars[1].type, child_vars[1].idx))
            elif node.op == 'Extract':
                opcode = ExpirOpcode.EXTRACT
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[0]))
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[1]))
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
            elif node.op == 'fpToSBV':
                opcode = ExpirOpcode.FP_TO_SBV
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[0]))
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[2])) 
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
            elif node.op == 'fpToFP':
                opcode = ExpirOpcode.FP_TO_FP
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[0]))
                args.append(ExpirArg(ExpirArgType.CONSTANT, node.args[2])) 
                args.append(ExpirArg(child_vars[0].type, child_vars[0].idx))
            else:
                raise ValueError(f"unhandled type {node.op}")
            
            ir_unsorted.append(ExpirNode(opcode, args))
            expr_to_id[node.hash()] = var_idx
            return IrWalkResult(var_idx, ExpirArgType.VAR)
        
    walk(expr)
    if len(ir_unsorted) == 0 and expr.op == 'BVV':
        ir_unsorted.append(ExpirNode(ExpirOpcode.LOAD_CONST, [
            ExpirArg(ExpirArgType.VAR, 0),
            ExpirArg(ExpirArgType.CONSTANT, expr.args[0])
        ]))

    seen_nodes = set()
    ir_sorted = []
    for ir in ir_unsorted:
        if ir not in seen_nodes:
            seen_nodes.add(ir)
            ir_sorted.append(ir)

    return ir_sorted

def print_ir(ir):
    for node in ir:
        str = f"{node.opcode} "
        for arg in node.args:
            if arg.type == ExpirArgType.HW_REG:
                str += f"${arg.value} "
            if arg.type == ExpirArgType.FUNC_ARGS:
                str += f"%{arg.value} "
            elif arg.type == ExpirArgType.VAR:
                str += f"#{arg.value} "
            else:
                str += f"{arg.value} "
        print(f"{str}")


class ExpirToLLVMCompiler:
    def __init__(self, target_bitwidth: int = 64):
        self.bitwidth = target_bitwidth
        self.i64 = ir.IntType(64)
        self.i8 = ir.IntType(8)
        
        self.module = ir.Module(name="expir_compiled_module")
        self.builder = None
        self.func = None
        
        self.vmap: Dict[int, ir.Value] = {}

        self.func_args_ptr = None

    def _get_llvm_arg(self, arg: ExpirArg) -> ir.Value:
        if arg.type == ExpirArgType.CONSTANT:
            print(f"constant: {arg.value}")
            return ir.Constant(self.i64, arg.value)

        elif arg.type == ExpirArgType.VAR:
            if arg.value not in self.vmap:
                raise ValueError(f"Variable idx {arg.value} referenced before definition.")
            return self.vmap[arg.value]
            
        elif arg.type == ExpirArgType.FUNC_ARGS:
            gv_idx = ir.Constant(self.i64, arg.value)
            ptr = self.builder.gep(self.func_args_ptr, [gv_idx], name=f"addr_fa_{arg.value}")
            return self.builder.ptrtoint(ptr, self.i64)
            
        raise NotImplementedError(f"Unsupported ExpirArgType: {arg.type}")

    def init_body(self, func_name: str = "execute_expr"):
        ptr_type = ir.PointerType(self.i64)
        fnty = ir.FunctionType(ir.VoidType(), [ptr_type, ptr_type])
        
        self.func = ir.Function(self.module, fnty, name=func_name)
        
        self.func.args[0].name = "hw_regs"
        self.func.args[1].name = "game_values"
        
        self.hw_regs_ptr = self.func.args[0]
        self.func_args_ptr = self.func.args[1]
        
        entry_block = self.func.append_basic_block(name="entry")
        self.builder = ir.IRBuilder(entry_block)

    def finish_body(self):
        if not self.builder.block.is_terminated:
            self.builder.ret_void()
        return self.module

    def compile_nodes(self, ir_nodes: List[ExpirNode]) -> ir.Module:
        self.vmap.clear()
        for node in ir_nodes:
            dest_var_idx = node.args[0].value

            if node.opcode == ExpirOpcode.ADD:
                left = self._get_llvm_arg(node.args[1])
                right = self._get_llvm_arg(node.args[2])
                self.vmap[dest_var_idx] = self.builder.add(left, right, name=f"add_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.SUB:
                left = self._get_llvm_arg(node.args[1])
                right = self._get_llvm_arg(node.args[2])
                self.vmap[dest_var_idx] = self.builder.sub(left, right, name=f"sub_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.XOR:
                left = self._get_llvm_arg(node.args[1])
                right = self._get_llvm_arg(node.args[2])
                self.vmap[dest_var_idx] = self.builder.xor(left, right, name=f"xor_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.MUL:
                left = self._get_llvm_arg(node.args[1])
                right = self._get_llvm_arg(node.args[2])
                self.vmap[dest_var_idx] = self.builder.mul(left, right, name=f"mul_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.INVERT:
                target = self._get_llvm_arg(node.args[1])
                all_ones = ir.Constant(self.i64, -1)
                self.vmap[dest_var_idx] = self.builder.xor(target, all_ones, name=f"not_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.LOAD:
                src_base = self._get_llvm_arg(node.args[1])
                ptr = self.builder.inttoptr(src_base, ir.PointerType(ir.IntType(64)), name=f"ptr_{dest_var_idx}")
                self.vmap[dest_var_idx] = self.builder.load(ptr, name=f"load_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.LOAD_CONST:
                self.vmap[dest_var_idx] = ir.Constant(self.i64, node.args[1].value)

            elif node.opcode == ExpirOpcode.STORE:
                bit_size = node.args[2].value
                dynamic_type = ir.IntType(bit_size)
                dynamic_ptr_type = ir.PointerType(dynamic_type)

                ptr = self.builder.inttoptr(self.vmap[node.args[1].value], dynamic_ptr_type, name=f"ptr_store")
                value_to_store = self.vmap[node.args[0].value]

                if value_to_store.type.width != bit_size:
                    if value_to_store.type.width > bit_size:
                        value_to_store = self.builder.trunc(value_to_store, dynamic_type, name="store_trunc")
                    else:
                        value_to_store = self.builder.zext(value_to_store, dynamic_type, name="store_zext")

                self.builder.store(value_to_store, ptr)

            elif node.opcode == ExpirOpcode.EXTRACT:
                high_bit = node.args[1].value
                low_bit = node.args[2].value
                
                intra_word_shift = low_bit % 64
                
                bit_width = (high_bit - low_bit) + 1
                mask_val = (1 << bit_width) - 1
                mask = ir.Constant(self.i64, mask_val)
                
                target = self._get_llvm_arg(node.args[3])
                base_ptr = self.builder.zext(target, self.i64)

                shift_amt = ir.Constant(self.i64, intra_word_shift)
                shifted = self.builder.lshr(base_ptr, shift_amt)
                
                self.vmap[dest_var_idx] = self.builder.and_(
                    shifted, mask, name=f"extract_{dest_var_idx}"
                )

            elif node.opcode == ExpirOpcode.SIGN_EXT:
                src_val = self._get_llvm_arg(node.args[1])
                self.vmap[dest_var_idx] = self.builder.sext(src_val, self.i64, name=f"sext_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.CONCAT:
                running_val = ir.Constant(self.i64, 0)
                for child_arg in node.args[1:]:
                    child_llvm = self._get_llvm_arg(child_arg)
                    running_val = self.builder.shl(running_val, ir.Constant(self.i64, 8)) 
                    running_val = self.builder.or_(running_val, child_llvm)
                self.vmap[dest_var_idx] = self.builder.bitcast(running_val, self.i64, name=f"concat_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.FP_TO_SBV:
                target_bit_size = node.args[2].value
                float_val = self._get_llvm_arg(node.args[3])

                target_int_type = ir.IntType(target_bit_size)
                self.vmap[dest_var_idx] = self.builder.fptosi(
                    float_val, target_int_type, name=f"fp_to_sbv_{dest_var_idx}"
                )

            elif node.opcode == ExpirOpcode.FP_TO_FP:
                target_sort = node.args[2].value
                
                if node.args[3].type == ExpirArgType.VAR:
                    src_float_val = self._get_llvm_arg(node.args[3])
                else:
                    fallback_idx = dest_var_idx - 1
                    if fallback_idx in self.vmap:
                        src_float_val = self.vmap[fallback_idx]
                    else:
                        raise KeyError(f"Could not resolve source float variable for instruction {dest_var_idx}")

                if hasattr(target_sort, 'size'):
                    target_width = target_sort.size() if callable(target_sort.size) else target_sort.size
                else:
                    target_str = str(target_sort).upper()
                    target_width = 64 if "DOUBLE" in target_str else 32

                if target_width == 32:
                    target_fp_type = ir.FloatType()
                elif target_width == 64:
                    target_fp_type = ir.DoubleType()
                else:
                    raise NotImplementedError(f"Unsupported float target size: {target_width}")

                if hasattr(src_float_val, 'type') and isinstance(src_float_val.type, ir.FloatType):
                    src_width = 32
                elif hasattr(src_float_val, 'type') and isinstance(src_float_val.type, ir.DoubleType):
                    src_width = 64
                else:
                    src_width = 64
                    src_float_val = self.builder.bitcast(src_float_val, ir.DoubleType(), name=f"bitcast_src_{dest_var_idx}")

                if src_width < target_width:
                    self.vmap[dest_var_idx] = self.builder.fpext(
                        src_float_val, target_fp_type, name=f"fp_extend_{dest_var_idx}")
                elif src_width > target_width:
                    self.vmap[dest_var_idx] = self.builder.fptrunc(
                        src_float_val, target_fp_type, name=f"fp_trunc_{dest_var_idx}")
                else:
                    self.vmap[dest_var_idx] = self.builder.bitcast(
                        src_float_val, target_fp_type, name=f"fp_copy_{dest_var_idx}")

            elif node.opcode == ExpirOpcode.INVALID:
                self.builder.unreachable()

def save_llvm_module_to_object_file(llvm_module, output_filename="output_expr.o"):
    llvm.initialize_native_target()
    llvm.initialize_native_asmprinter()

    llvm_ir_string = str(llvm_module)
    native_mod = llvm.parse_assembly(llvm_ir_string)
    native_mod.verify()

    target_triple = llvm.get_process_triple()
    target = llvm.Target.from_triple(target_triple)
    target_machine = target.create_target_machine()

    object_code_bytes = target_machine.emit_object(native_mod)

    with open(output_filename, "wb") as f:
        f.write(object_code_bytes)

    print(f"[+] Successfully compiled and saved native object file to: {output_filename}")

class Analyzer:
    def init_angr(self, pe_path):
        self.project = angr.Project(
            pe_path,
            main_opts={
                'backend': 'elf',
                'arch': 'amd64',
                'base_addr': 0x0,
                'entry_point': 0x162BA0
            },
            auto_load_libs=True)

        state = self.project.factory.blank_state(addr=0x1624F0)

        # this is simply for speed
        state.options.add(angr.options.UNICORN)
        state.options.add(angr.options.UNICORN_SYM_REGS_SUPPORT)

        # not sure if this is required
        state.options.add(angr.options.SYMBOLIC_WRITE_ADDRESSES)

        # we want tracking, so we can fully run our simulation
        # and then pull out what we want
        state.options.add(angr.options.TRACK_MEMORY_ACTIONS)
        state.options.add(angr.options.TRACK_ACTION_HISTORY)
        return state

    def setup_engine(self, state):
        """
        Sets up engine memory for the packet construction.

        The engine itself isn't specifically used, so we symbolize child pointers
        which are used.
        """

        self.engine_194a8_addr = state.heap.allocate(0x10000)
        state.memory.store(self.engine_194a8_addr + 0x490, claripy.BVS("engine_194a8_490#", 32), endness=state.arch.memory_endness)
        
        self.engine_addr = state.heap.allocate(0x20000)
        state.memory.store(self.engine_addr + 0x19b30, claripy.BVS("engine_19b30#", 64), endness=state.arch.memory_endness)
        state.memory.store(self.engine_addr + 0x194a8, claripy.BVV(self.engine_194a8_addr, 64), endness=state.arch.memory_endness)
        
    def setup_menu(self, state):
        """
        Sets up menu memory for the packet construction.

        Specifically we symbolize the menu actions which are directly put into the packet
        after obfuscation.
        """
        self.menu_args_addr = state.heap.allocate(0x100)
        state.memory.store(self.menu_args_addr + 0x48, claripy.BVS("menu_action_0#", 32), endness=state.arch.memory_endness)
        state.memory.store(self.menu_args_addr + 0x4c, claripy.BVS("menu_action_1#", 32), endness=state.arch.memory_endness)
        state.memory.store(self.menu_args_addr + 0x50, claripy.BVS("menu_action_2#", 32), endness=state.arch.memory_endness)
        state.memory.store(self.menu_args_addr + 0x54, claripy.BVS("menu_action_3#", 32), endness=state.arch.memory_endness)

        self.menu_action_ctx_addr = state.heap.allocate(0x200)
        state.memory.store(self.menu_action_ctx_addr + 0x8, claripy.BVV(self.menu_args_addr, 64), endness=state.arch.memory_endness)

        self.menu_action_addr = state.heap.allocate(0x200)
        state.memory.store(self.menu_action_addr, self.engine_addr, endness=state.arch.memory_endness)

    def apply_hooks(self, state):
        class Function_19A420_Hook(SimProcedure):
            def run(self):
                dummy_ptr3 = self.state.heap.allocate(0x400)
                self.state.memory.store(dummy_ptr3 + 0xd0, claripy.FPS("unknown_1#", claripy.fp.FSORT_FLOAT), endness=state.arch.memory_endness)
                self.state.memory.store(dummy_ptr3 + 0xd4, claripy.FPS("unknown_2#", claripy.fp.FSORT_FLOAT), endness=state.arch.memory_endness)

                dummy_ptr2 = self.state.heap.allocate(0x10)
                self.state.memory.store(dummy_ptr2 + 0x8, dummy_ptr3, endness=state.arch.memory_endness)
                
                dummy_ptr1 = self.state.heap.allocate(0x10)
                self.state.memory.store(dummy_ptr1 + 0x8, dummy_ptr2, endness=state.arch.memory_endness)
                return dummy_ptr1

        class Return1Hook(SimProcedure):
            def run(self):
                return 1
            
        class AllocatePacketHook(SimProcedure):
            def run(self, output):
                buffer = self.state.heap.allocate(0x40)
                body = claripy.BVS("packet_buffer#", 64)

                self.state.globals["packet_body"] = body

                self.state.memory.store(buffer + 0x18, body, endness=state.arch.memory_endness)
                self.state.memory.store(buffer + 0x20, claripy.BVV(0, 64), endness=state.arch.memory_endness)

                self.state.memory.store(output + 0x8, claripy.BVV(buffer, 64), endness=state.arch.memory_endness)
                return buffer

        # this function provides memory used in
        # packet data, so we need to symbolize it
        self.project.hook(0x19a420, Function_19A420_Hook())

        # we need to symbolize the packet buffer
        self.project.hook(0x1378d0, AllocatePacketHook())

        # these are primarily to stop angr from 
        # running for so long, but should not be required
        self.project.hook(0x230f40, Return1Hook())
        self.project.hook(0x161c10, Return1Hook())
        self.project.hook(0xcdad60, Return1Hook())
        self.project.hook(0x280220, Return1Hook())

    def apply_expr_caches(self, state):
        def bind_unconstrained_reads(state):
            if state.globals.get("memory_load_registry") == None:
                state.globals["memory_load_registry"] = {}

            addr_expr = state.inspect.mem_read_address
            read_expr = state.inspect.mem_read_expr
            node_hash = read_expr.hash()

            access = DataAccess(node_hash, state.solver.symbolic(addr_expr), addr_expr)
            state.globals["memory_load_registry"][node_hash] = access

        def bind_unconstrained_writes(state: SimState):
            if state.globals.get("memory_store_registry") == None:
                state.globals["memory_store_registry"] = {}

            addr_expr = state.inspect.mem_write_address
            read_expr = state.inspect.mem_write_expr
            node_hash = read_expr.hash()

            access = DataAccess(node_hash, state.solver.symbolic(addr_expr), addr_expr)
            state.globals["memory_store_registry"][node_hash] = access

        state.inspect.b('mem_read', when=angr.BP_AFTER, action=bind_unconstrained_reads)
        state.inspect.b('mem_write', when=angr.BP_AFTER, action=bind_unconstrained_writes)

    def extract_packet_writes(self):
        all_final_states = [s.state if hasattr(s, 'state') else s for s in (
            self.simgr.deadended + self.simgr.active + self.simgr.errored)]

        count = 0
        for _, state in enumerate(all_final_states):
            actions = state.history.actions
            packet_body = state.globals.get("packet_body")
            if packet_body != None:
                compiler = ExpirToLLVMCompiler()
                compiler.init_body()
                for action in actions:
                    if action.type == 'mem' and action.action == 'write':
                        addr_ast = action.addr.ast
                        data_ast = action.data.ast

                        size_in_bits = action.size.ast
                        if not isinstance(size_in_bits, int):
                            size_in_bits = state.solver.eval(size_in_bits)

                        target_variable_name = packet_body.args[0] 

                        address_variables = addr_ast.variables
                        print(f"here: {target_variable_name} {addr_ast}")

                        if target_variable_name in address_variables:
                            load_expr = compile_expr_to_ir(state.globals["memory_load_registry"], 
                                                           state.globals["memory_store_registry"],
                                                           claripy.simplify(data_ast))
                            
                            store_expr = compile_expr_to_ir(state.globals["memory_load_registry"], 
                                                            state.globals["memory_store_registry"], 
                                                            claripy.simplify(addr_ast))

                            # both of the traces have overlapping variables
                            # so we offset the 2nd expression's variable numbers
                            for node in store_expr:
                                for arg in node.args:
                                    if arg.type == ExpirArgType.VAR:
                                        arg.value += len(load_expr)

                            # combine both the load + store and then we
                            # add our store operation because it's not tracked
                            # in our write cache for some reason..
                            all_expr = load_expr + store_expr
                            all_expr.append(ExpirNode(ExpirOpcode.STORE, [
                                ExpirArg(ExpirArgType.VAR, len(load_expr) - 1),
                                ExpirArg(ExpirArgType.VAR, len(load_expr) + len(store_expr) - 1),
                                ExpirArg(ExpirArgType.CONSTANT, size_in_bits)
                            ]))

                            # compile into our lifted function
                            compiler.compile_nodes(all_expr)

                llvm_module = compiler.finish_body()
                print(str(llvm_module))
                save_llvm_module_to_object_file(llvm_module, f"{count}.o")
                count += 1

    def setup_emulation(self, pe_path):
        state = self.init_angr(pe_path)
        self.setup_engine(state)
        self.setup_menu(state)
        
        state.regs.rsp = 0x7ffffffffff0000 
        state.regs.fs = 0x20000000
        state.regs.gs = 0x30000000
        state.regs.rdi = claripy.BVV(self.menu_action_addr, 64)
        state.regs.rsi = claripy.BVV(self.menu_action_ctx_addr, 64)

        self.apply_hooks(state)
        self.apply_expr_caches(state)

        self.simgr = self.project.factory.simulation_manager(state)
        while self.simgr.active:
            self.simgr.step()

        self.extract_packet_writes()
    
if __name__ == "__main__":
    a = Analyzer()
    a.setup_emulation("rs2client")
