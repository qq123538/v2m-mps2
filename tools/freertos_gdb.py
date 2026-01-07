import gdb

# --- Helper Class for Common Operations ---
class FreeRTOSHelper:
    """Shared helpers for type lookups and list traversal."""
    
    @staticmethod
    def get_type(type_name):
        try:
            return gdb.lookup_type(type_name)
        except:
            return None

    @staticmethod
    def get_tcb_type():
        return FreeRTOSHelper.get_type("struct tskTaskControlBlock").pointer()

    @staticmethod
    def get_list_item_type():
        return FreeRTOSHelper.get_type("ListItem_t").pointer()

    @staticmethod
    def get_queue_type():
        # Usually 'struct QueueDefinition' or 'xQUEUE'
        t = FreeRTOSHelper.get_type("struct QueueDefinition")
        if not t: t = FreeRTOSHelper.get_type("xQUEUE")
        return t.pointer() if t else None

    @staticmethod
    def get_timer_type():
        # Usually 'struct tmrTimerControl' or 'xTIMER'
        t = FreeRTOSHelper.get_type("struct tmrTimerControl")
        if not t: t = FreeRTOSHelper.get_type("xTIMER")
        return t.pointer() if t else None

    @staticmethod
    def is_scheduler_running():
        """Checks if FreeRTOS scheduler is likely running."""
        try:
            # Primary check: pxCurrentTCB should not be NULL
            tcb = gdb.parse_and_eval("pxCurrentTCB")
            if int(tcb) == 0:
                return False
            return True
        except:
            return False

    @staticmethod
    def walk_list(list_ptr, callback):
        """
        Walks a FreeRTOS List_t.
        list_ptr: gdb.Value (pointer to List_t or List_t)
        callback: function(owner_ptr) -> None
        """
        if not list_ptr: return

        if list_ptr.type.code == gdb.TYPE_CODE_PTR:
            the_list = list_ptr.dereference()
        else:
            the_list = list_ptr

        try:
            count = int(the_list['uxNumberOfItems'])
        except:
            return

        if count == 0:
            return

        list_end_addr = the_list['xListEnd'].address
        curr_node_ptr = the_list['xListEnd']['pxNext']
        list_item_type = FreeRTOSHelper.get_list_item_type()

        for _ in range(count):
            if curr_node_ptr == list_end_addr:
                break
            
            curr_node = curr_node_ptr.cast(list_item_type)
            owner = curr_node['pvOwner']
            
            if owner:
                callback(owner)
            
            curr_node_ptr = curr_node['pxNext']

# --- Main Command (Prefix) ---
class FreeRTOS(gdb.Command):
    """
    FreeRTOS Kernel Inspection Tools.
    Subcommands:
      freertos tasks   : List all tasks
      freertos bt      : Backtrace a task (usage: bt <name|addr|all>)
      freertos queues  : List registered queues (requires configQUEUE_REGISTRY_SIZE > 0)
      freertos timers  : List active software timers
    """
    def __init__(self):
        super(FreeRTOS, self).__init__("freertos", gdb.COMMAND_USER, gdb.COMPLETE_COMMAND, True)

# --- Subcommand: Tasks ---
class FreeRTOSTasks(gdb.Command):
    """List all FreeRTOS tasks."""
    def __init__(self):
        super(FreeRTOSTasks, self).__init__("freertos tasks", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        if not FreeRTOSHelper.is_scheduler_running():
            print("FreeRTOS scheduler does not appear to be running (pxCurrentTCB is NULL).")
            return

        tcb_type = FreeRTOSHelper.get_tcb_type()
        if not tcb_type:
            print("Error: Could not find tskTaskControlBlock type.")
            return

        try:
            current_tcb = gdb.parse_and_eval("pxCurrentTCB")
        except:
            print("Error: pxCurrentTCB not found.")
            return

        tasks = {}

        def collect_task(owner_ptr, state):
            tcb = owner_ptr.cast(tcb_type)
            addr = str(tcb).split()[0]
            if addr in tasks: return

            try: name = tcb['pcTaskName'].string()
            except: name = "???"
            
            try: prio = int(tcb['uxPriority'])
            except: prio = -1
            
            try: stack = tcb['pxTopOfStack']
            except: stack = 0

            tasks[addr] = {
                'name': name,
                'prio': prio,
                'state': state,
                'stack': stack,
                'running': (tcb == current_tcb)
            }

        # Scan Lists
        try:
            ready_lists = gdb.parse_and_eval("pxReadyTasksLists")
            num = ready_lists.type.range()[1] + 1
            for i in range(num):
                FreeRTOSHelper.walk_list(ready_lists[i], lambda o: collect_task(o, "Ready"))
        except: pass

        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxDelayedTaskList"), lambda o: collect_task(o, "Blocked"))
        except: pass
        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxOverflowDelayedTaskList"), lambda o: collect_task(o, "Blocked"))
        except: pass
        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("xSuspendedTaskList"), lambda o: collect_task(o, "Suspended"))
        except: pass
        
        # Ensure running task is there
        collect_task(current_tcb, "Running")

        # Print
        print(f"{ 'Address':<18} {'Name':<16} {'Prio':<6} {'State':<12} {'Stack Top'}")
        print("-" * 75)
        for addr, info in sorted(tasks.items(), key=lambda x: x[1]['prio'], reverse=True):
            state = info['state']
            if info['running']: state = "*Running"
            print(f"{addr:<18} {info['name']:<16} {info['prio']:<6} {state:<12} {info['stack']}")


# --- Subcommand: Backtrace ---
class FreeRTOSBacktrace(gdb.Command):
    """
    Show backtrace of a FreeRTOS task.
    Usage: freertos bt <task_name | task_address | all>
    """
    def __init__(self):
        super(FreeRTOSBacktrace, self).__init__("freertos bt", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        if not FreeRTOSHelper.is_scheduler_running():
            print("FreeRTOS scheduler does not appear to be running (pxCurrentTCB is NULL).")
            return

        if not arg:
            print("Usage: freertos bt <name|addr|all>")
            return

        tcb_type = FreeRTOSHelper.get_tcb_type()
        current_tcb = gdb.parse_and_eval("pxCurrentTCB")

        if arg.lower() == 'all':
            self.bt_all(tcb_type, current_tcb)
        else:
            # Find specific
            target = self.find_task(arg, tcb_type, current_tcb)
            if target:
                self.do_bt(target, current_tcb)
            else:
                print(f"Task '{arg}' not found.")

    def find_task(self, query, tcb_type, current_tcb):
        # Check if address
        try:
            addr = int(query, 0)
            return gdb.Value(addr).cast(tcb_type)
        except:
            pass
        
        # Scan for name
        found = None
        def check(owner):
            nonlocal found
            if found: return
            tcb = owner.cast(tcb_type)
            try:
                if tcb['pcTaskName'].string() == query:
                    found = tcb
            except: pass

        # Scan lists (reuse logic conceptually, simplified here)
        # For brevity, I'll just check current, then scan ready lists
        try:
            if current_tcb['pcTaskName'].string() == query: return current_tcb
        except: pass
        
        try:
            ready = gdb.parse_and_eval("pxReadyTasksLists")
            for i in range(ready.type.range()[1] + 1):
                FreeRTOSHelper.walk_list(ready[i], check)
                if found: return found
        except: pass
        
        # Check blocked
        if not found: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxDelayedTaskList"), check)
        if not found: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxOverflowDelayedTaskList"), check)
        
        return found

    def bt_all(self, tcb_type, current_tcb):
        found = set()
        def collect(owner):
            tcb = owner.cast(tcb_type)
            addr = int(tcb)
            if addr not in found:
                found.add(addr)
                self.do_bt(tcb, current_tcb)
                print("\n" + "-"*40 + "\n")
        
        collect(current_tcb)
        # Scan lists
        try:
            ready = gdb.parse_and_eval("pxReadyTasksLists")
            for i in range(ready.type.range()[1] + 1):
                FreeRTOSHelper.walk_list(ready[i], collect)
        except: pass
        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxDelayedTaskList"), collect)
        except: pass
        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("pxOverflowDelayedTaskList"), collect)
        except: pass
        try: FreeRTOSHelper.walk_list(gdb.parse_and_eval("xSuspendedTaskList"), collect)
        except: pass

    def do_bt(self, tcb, current_tcb):
        try:
            try: name = tcb['pcTaskName'].string()
            except: name = "???"
            print(f"Backtrace for Task: {name} ({tcb})")

            if tcb == current_tcb:
                gdb.execute("bt")
                return

            # Unwind logic (CM4/CM7 specific)
            stack_ptr = tcb['pxTopOfStack']
            
            # Try standard registers. If xpsr/fpscr fails, we skip it.
            regs_to_save = ["r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","sp","lr","pc","xpsr","fpscr"]
            saved = {}
            for r in regs_to_save:
                try:
                    # Cast to int immediately to store raw value. 
                    # This prevents storing "0xAddr <Symbol>" strings which break 'set' commands.
                    val = gdb.parse_and_eval(f"${r}")
                    saved[r] = int(val)
                except:
                    pass # Skip registers we can't read

            try:
                # Layout: R4-R11 (8 words) + EXC_RETURN (1 word)? 
                # Re-checking port.c logic: 
                # r0 = pxTopOfStack
                # ldmia r0!, {r4-r11, r14}  -> Pops 9 words. 
                # Then MSR PSP, r0 -> SP is now +36 bytes.
                
                # The context on stack at 'stack_ptr':
                # [R4, R5, R6, R7, R8, R9, R10, R11, LR(EXC_RET)]
                # followed by HW frame: [R0, R1, R2, R3, R12, LR, PC, xPSR]
                
                # For GDB to unwind, we usually need SP, PC, LR, R7.
                
                # 1. Calculate the 'hardware' stack pointer (after software context restore)
                sw_context_size = 9 * 4 # 9 registers * 4 bytes
                hw_sp = int(stack_ptr) + sw_context_size
                
                # 2. Extract Key Registers from SW Context
                # R7 is at index 3 (0-based) in {r4..r11} -> stack_ptr + 3*4
                # We use gdb.parse_and_eval with a cast to read memory directly
                # Note: We must be careful with the expression syntax.
                sw_r7_val = gdb.parse_and_eval(f"*(unsigned int*)({int(stack_ptr)} + 12)")
                
                # 3. Extract Key Registers from HW Context
                # HW Frame: R0(0), R1(1), R2(2), R3(3), R12(4), LR(5), PC(6), xPSR(7)
                # Addrs are relative to hw_sp
                hw_lr_val = gdb.parse_and_eval(f"*(unsigned int*)({hw_sp} + 20)")
                hw_pc_val = gdb.parse_and_eval(f"*(unsigned int*)({hw_sp} + 24)")
                
                # 4. Apply
                gdb.execute(f"set $sp = {hw_sp}")
                gdb.execute(f"set $r7 = {sw_r7_val}")
                gdb.execute(f"set $lr = {hw_lr_val}")
                gdb.execute(f"set $pc = {hw_pc_val}")
                
                gdb.execute("bt")
                
            except Exception as e:
                msg = str(e)
                # Suppress known harmless symbol error from environment
                if "uxIdleTaskStack" in msg or "No symbol" in msg:
                    # print(f"(Supressed GDB error: {msg})")
                    pass
                else:
                    print(f"Error unwinding: {e}")
            finally:
                self.restore_regs(saved)
                # Important: Tell GDB to re-read registers from the target
                # to ensure its internal cache is consistent before 'continue'
                gdb.execute("maintenance flush register-cache")

        except Exception as outer_e:
             print(f"Critical error in do_bt: {outer_e}")

    def restore_regs(self, saved):
        for r, val in saved.items():
            try:
                # Use hex() to ensure it's treated as a value, though int works too.
                gdb.execute(f"set ${r} = {val}")
            except Exception as e:
                print(f"Warning: Failed to restore register ${r}: {e}")


# --- Subcommand: Queues ---
class FreeRTOSQueues(gdb.Command):
    """
    List registered FreeRTOS queues.
    Requires configQUEUE_REGISTRY_SIZE > 0 and queues added to registry.
    """
    def __init__(self):
        super(FreeRTOSQueues, self).__init__("freertos queues", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        if not FreeRTOSHelper.is_scheduler_running():
            print("FreeRTOS scheduler does not appear to be running (pxCurrentTCB is NULL).")
            return

        try:
            registry = gdb.parse_and_eval("xQueueRegistry")
            # Determine size
            reg_size = registry.type.range()[1] + 1
        except:
            print("Error: xQueueRegistry not found. Ensure configQUEUE_REGISTRY_SIZE > 0.")
            return

        print(f"{ 'Name':<20} {'Handle':<18} {'Waiting':<8} {'Length':<8} {'ItemSize'}")
        print("-" * 75)

        q_type = FreeRTOSHelper.get_queue_type()

        for i in range(reg_size):
            item = registry[i]
            try:
                name_ptr = item['pcQueueName']
                if int(name_ptr) == 0: continue
                name = name_ptr.string()
                
                handle = item['xHandle']
                if int(handle) == 0: continue
                
                # Cast to Queue_t to read details
                q = handle.cast(q_type)
                waiting = int(q['uxMessagesWaiting'])
                length = int(q['uxLength'])
                item_size = int(q['uxItemSize'])
                
                print(f"{name:<20} {str(handle):<18} {waiting:<8} {length:<8} {item_size}")
                
            except Exception as e:
                # print(e)
                pass

# --- Subcommand: Timers ---
class FreeRTOSTimers(gdb.Command):
    """List active FreeRTOS software timers."""
    def __init__(self):
        super(FreeRTOSTimers, self).__init__("freertos timers", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        if not FreeRTOSHelper.is_scheduler_running():
            print("FreeRTOS scheduler does not appear to be running (pxCurrentTCB is NULL).")
            return

        # Timers are in pxCurrentTimerList and pxOverflowTimerList
        # These are usually static in timers.c
        # We need to try finding them.
        
        timer_type = FreeRTOSHelper.get_timer_type()
        if not timer_type:
            print("Error: Timer type not found.")
            return

        lists = []
        try: lists.append(gdb.parse_and_eval("'timers.c'::pxCurrentTimerList"))
        except: 
            try: lists.append(gdb.parse_and_eval("pxCurrentTimerList"))
            except: pass
            
        try: lists.append(gdb.parse_and_eval("'timers.c'::pxOverflowTimerList"))
        except:
             try: lists.append(gdb.parse_and_eval("pxOverflowTimerList"))
             except: pass

        if not lists:
            print("Error: Timer lists not found (check configUSE_TIMERS=1 and symbols).")
            return

        print(f"{'Timer':<45} {'ID':<15} {'PeriodInMs':<14} {'Overflow':<10} {'Status':<25} {'Callback'}")
        print("-" * 130)

        def print_timer(owner, is_overflow):
            tmr = owner.cast(timer_type)
            addr = str(tmr).split()[0]
            try: name = tmr['pcTimerName'].string()
            except: name = "???"
            
            # Combined Timer Column
            timer_display = f"{name} ({addr})"

            try: period = int(tmr['xTimerPeriodInTicks'])
            except: period = 0

            # Timer ID
            try:
                # pvTimerID is void*, show as pure hex to avoid misleading symbol lookups
                timer_id_val = int(tmr['pvTimerID'])
                timer_id = hex(timer_id_val)
            except:
                timer_id = "?"
            
            # ucStatus decoding
            # bit 0 (0x01) : Active
            # bit 1 (0x02) : Statically Allocated
            # bit 2 (0x04) : AutoReload
            status_str = []
            try: 
                status = int(tmr['ucStatus'])
                if status & 0x01: status_str.append("Active")
                if status & 0x02: status_str.append("Static")
                if status & 0x04: status_str.append("AutoReload")
            except: 
                status_str.append("?")
            
            status_display = ",".join(status_str)

            # Callback symbol resolution
            callback_name = "???"
            try:
                # Cast to int to get raw address, avoiding "0x... <Symbol>" string issues in gdb command
                callback_ptr = int(tmr['pxCallbackFunction'])
                # Use gdb 'info symbol' to get the name
                res = gdb.execute(f"info symbol {callback_ptr}", to_string=True)
                # Output format: "vTimerCallback in section .text"
                callback_name = res.split()[0]
            except:
                pass

            overflow_str = "Yes" if is_overflow else "No"
            
            print(f"{timer_display:<45} {timer_id:<15} {period:<14} {overflow_str:<10} {status_display:<25} {callback_name}")

        # Iterate lists with context
        # lists[0] is typically pxCurrentTimerList (Normal)
        # lists[1] is typically pxOverflowTimerList (Overflow)
        
        # We need to match the list lookup logic to pass the correct flag
        # Re-evaluating logic to be robust:
        
        current_list = None
        overflow_list = None

        try: current_list = gdb.parse_and_eval("'timers.c'::pxCurrentTimerList")
        except: 
            try: current_list = gdb.parse_and_eval("pxCurrentTimerList")
            except: pass
            
        try: overflow_list = gdb.parse_and_eval("'timers.c'::pxOverflowTimerList")
        except:
             try: overflow_list = gdb.parse_and_eval("pxOverflowTimerList")
             except: pass

        if current_list:
             FreeRTOSHelper.walk_list(current_list, lambda o: print_timer(o, False))
        
        if overflow_list:
             FreeRTOSHelper.walk_list(overflow_list, lambda o: print_timer(o, True))

# Registration
FreeRTOS()
FreeRTOSTasks()
FreeRTOSBacktrace()
FreeRTOSQueues()
FreeRTOSTimers()

print("FreeRTOS GDB Inspector loaded.")
print("Commands: freertos [tasks | bt | queues | timers]")
