/*
 * Copyright 2015, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

/*
 * seL4 tutorial part 3: IPC between 2 threads
 */

/* Include Kconfig variables. */
#include <autoconf.h>

#include <stdio.h>
#include <assert.h>

#include <sel4/sel4.h>
#include <sel4/types_gen.h>

#include <simple/simple.h>
#include <simple-default/simple-default.h>

#include <vka/object.h>
#include <vka/object_capops.h>

#include <allocman/allocman.h>
#include <allocman/bootstrap.h>
#include <allocman/vka.h>

#include <vspace/vspace.h>

#include <sel4utils/vspace.h>
#include <sel4utils/mapping.h>

#include <utils/zf_log.h>
#include <sel4utils/sel4_zf_logif.h>

/* constants */
#define IPCBUF_FRAME_SIZE_BITS 12 // use a 4K frame for the IPC buffer
#define IPCBUF_VADDR 0x7000000 // arbitrary (but free) address for IPC buffer

#define EP_BADGE 0x61 // arbitrary (but unique) number for a badge
#define MSG_DATA 0x6161 // arbitrary data to send

/* global environment variables */
seL4_BootInfo *info;
simple_t simple;
vka_t vka;
allocman_t *allocman;

/* variables shared with second thread */
vka_object_t ep_object;
cspacepath_t ep_cap_path;

/* static memory for the allocator to bootstrap with */
#define ALLOCATOR_STATIC_POOL_SIZE ((1 << seL4_PageBits) * 10)
UNUSED static char allocator_mem_pool[ALLOCATOR_STATIC_POOL_SIZE];

/* stack for the new thread */
#define THREAD_2_STACK_SIZE 512
static uint64_t thread_2_stack[THREAD_2_STACK_SIZE];

/* convenience function */
extern void name_thread(seL4_CPtr tcb, char *name);

/* function to run in the new thread */
void thread_2(void) {
    seL4_Word sender_badge;
    seL4_MessageInfo_t tag;
    seL4_Word msg;

    printf("thread_2: hallo wereld\n");

    /* TODO 11: wait for a message to come in over the endpoint */
    /* hint 1: seL4_Recv()
     * seL4_MessageInfo_t seL4_Recv(seL4_CPtr src, seL4_Word* sender)
     * @param src The capability to be invoked.
     * @param sender The badge of the endpoint capability that was invoked by the sender is written to this address.
     * @return A seL4_MessageInfo_t structure
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_11:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 2: seL4_MessageInfo_t is generated during build.
     * The type definition and generated field access functions are defined in a generated file:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_11:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */

    /* TODO 12: make sure it is what we expected */
    /* hint 1: check the badge. is it EP_BADGE?
     * hint 2: we are expecting only 1 message register
     * hint 3: seL4_MessageInfo_get_length()
     * seL4_Uint32 CONST seL4_MessageInfo_get_length(seL4_MessageInfo_t seL4_MessageInfo)
     * @param seL4_MessageInfo the seL4_MessageInfo_t to extract a field from
     * @return the number of message registers delivered
     * seL4_MessageInfo_get_length() is generated during build. It can be found in:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_12:
     */
    ZF_LOGF_IF(sender_badge != EP_BADGE,
        "Badge on the endpoint was not what was expected.\n");

    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1,
        "Length of the data send from root thread was not what was expected.\n"
        "\tHow many registers did you set with seL4_SetMR, within the root thread?\n");

    /* TODO 13: get the message stored in the first message register */
    /* hint: seL4_GetMR()
     * seL4_Word seL4_GetMR(int i)
     * @param i The message register to retreive
     * @return The message register value
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_13:
     * You can find out more about message registers in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */

    printf("thread_2: got a message %#x from %#x\n", msg, sender_badge);

    /* modify the message */
    msg = ~msg;

    /* TODO 14: copy the modified message back into the message register */
    /* hint: seL4_SetMR()
     * void seL4_SetMR(int i, seL4_Word mr)
     * @param i The message register to write
     * @param mr The value of the message register
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_14:
     * You can find out more about message registers in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */

    /* TODO 15: send the message back */
    /* hint 1: seL4_ReplyRecv()
     * seL4_MessageInfo_t seL4_ReplyRecv(seL4_CPtr dest, seL4_MessageInfo_t msgInfo, seL4_Word *sender)
     * @param dest The capability to be invoked.
     * @param msgInfo The messageinfo structure for the IPC.  This specifies information about the message to send (such as the number of message registers to send) as the Reply part.
     * @param sender The badge of the endpoint capability that was invoked by the sender is written to this address.  This is a result of the Wait part.
     * @return A seL4_MessageInfo_t structure.  This is a result of the Wait part.
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_15:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 2: seL4_MessageInfo_t is generated during build.
     * The type definition and generated field access functions are defined in a generated file:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_15:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */
}

int main(void)
{
    UNUSED int error;

    /* Set up logging and give us a name: useful for debugging if the thread faults */
    zf_log_set_tag_prefix("hello-3:");
    name_thread(seL4_CapInitThreadTCB, "hello-3");

    /* get boot info */
    info = seL4_GetBootInfo();

    /* init simple */
    simple_default_init_bootinfo(&simple, info);

    /* print out bootinfo and other info about simple */
    simple_print(&simple);

    /* create an allocator */
    allocman = bootstrap_use_current_simple(&simple, ALLOCATOR_STATIC_POOL_SIZE, allocator_mem_pool);

    ZF_LOGF_IF(allocman == NULL, "Failed to initialize alloc manager.\n"
        "\tMemory pool sufficiently sized?\n"
        "\tMemory pool pointer valid?\n");

    /* create a vka (interface for interacting with the underlying allocator) */
    allocman_make_vka(&vka, allocman);

    /* get our cspace root cnode */
    seL4_CPtr cspace_cap;
    cspace_cap = simple_get_cnode(&simple);

    /* get our vspace root page directory */
    seL4_CPtr pd_cap;
    pd_cap = simple_get_pd(&simple);

    /* create a new TCB */
    vka_object_t tcb_object = {0};
    error = vka_alloc_tcb(&vka, &tcb_object);
    ZF_LOGF_IFERR(error, "Failed to allocate new TCB.\n"
        "\tVKA given sufficient bootstrap memory?");

    /*
     * create and map an ipc buffer:
     */

    /* TODO 1: get a frame cap for the ipc buffer */
    /* hint: vka_alloc_frame()
     * int vka_alloc_frame(vka_t *vka, uint32_t size_bits, vka_object_t *result)
     * @param vka Pointer to vka interface.
     * @param size_bits Frame size: 2^size_bits
     * @param result Structure for the Frame object.  This gets initialised.
     * @return 0 on success
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_1:
     */
    vka_object_t ipc_frame_object;
    error = vka_alloc_frame(vka, IPC_BUFFER_SIZE_BITS, ipc_frame_object);

    ZF_LOGF_IFERR(error, "Failed to allocate ipc frame object\n"
        "\tVKA given sufficient bootstrap memory?");

    /*
     * map the frame into the vspace at ipc_buffer_vaddr.
     * To do this we first try to map it in to the root page directory.
     * If there is already a page table mapped in the appropriate slot in the
     * page diretory where we can insert this frame, then this will succeed.
     * Otherwise we first need to create a page table, and map it in to
     * the page directory, before we can map the frame in. */

    seL4_Word ipc_buffer_vaddr = IPCBUF_VADDR;

    /* TODO 2: try to map the frame the first time  */
    /* hint 1: seL4_ARCH_Page_Map()
     * The *ARCH* versions of seL4 sys calls are abstractions over the architecture provided by libsel4utils
     * this one is defined as:
     * #define seL4_ARCH_Page_Map seL4_X86_Page_Map
     * in: Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_2:
     * The signature for the underlying function is:
     * int seL4_X86_Page_Map(seL4_X86_Page service, seL4_X86_PageDirectory pd, seL4_Word vaddr, seL4_CapRights rights, seL4_X86_VMAttributes attr)
     * @param service Capability to the page to map.
     * @param pd Capability to the VSpace which will contain the mapping.
     * @param vaddr Virtual address to map the page into.
     * @param rights Rights for the mapping.
     * @param attr VM Attributes for the mapping.
     * @return 0 on success.
     *
     * Note: this function is generated during build.  It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_2:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 2: for the rights, use seL4_AllRights
     * hint 3: for VM attributes use seL4_ARCH_Default_VMAttributes
     * Hint 4: It is normal for this function call to fail. That means there are
     *	no page tables with free slots -- proceed to the next step where you'll
     *	be led to allocate a new empty page table and map it into the VSpace,
     *	before trying again.
     */

    error = seL4_X86_Page_Map(ipc_frame_buffer.cptr, pd, vaddr, seL4_AllRights, seL4_ARCH_VMAttributes);

    ZF_LOGF_IFERR(error, "Failed to map page.\n");

    if (error != 0) {
        /* TODO 3: create a page table */
        /* hint: vka_alloc_page_table()
		 * int vka_alloc_page_table(vka_t *vka, vka_object_t *result)
		 * @param vka Pointer to vka interface.
		 * @param result Structure for the PageTable object.  This gets initialised.
		 * @return 0 on success
		 * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_3:
         */
        vka_object_t page_table_object;
		error = vka_alloc_page_table(vka, page_table_object);

        ZF_LOGF_IFERR(error, "Failed to allocate new page table.\n");

        /* TODO 4: map the page table */
        /* hint 1: seL4_ARCH_PageTable_Map()
		 * The *ARCH* versions of seL4 sys calls are abstractions over the architecture provided by libsel4utils
		 * this one is defined as:
		 * #define seL4_ARCH_PageTable_Map seL4_X86_PageTable_Map
         * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_4:
		 * The signature for the underlying function is:
		 * int seL4_X86_PageTable_Map(seL4_X86_PageTable service, seL4_X86_PageDirectory pd, seL4_Word vaddr, seL4_X86_VMAttributes attr)
		 * @param service Capability to the page table to map.
		 * @param pd Capability to the VSpace which will contain the mapping.
		 * @param vaddr Virtual address to map the page table into.
		 * @param rights Rights for the mapping.
		 * @param attr VM Attributes for the mapping.
		 * @return 0 on success.
		 *
		 * Note: this function is generated during build.  It is generated from the following definition:
         * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_4:
		 * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
		 *
         * hint 2: for VM attributes use seL4_ARCH_Default_VMAttributes
         */
		 error = seL4_X86_PageTable_Map(page_table_object.cptr, seL4_X86_PageDirectory pd, ipc_buffer_vaddr, seL4_ARCH_Default_VMAttributes);

        ZF_LOGF_IFERR(error, "Failed to map page table into VSpace.\n"
            "\tWe are inserting a new page table into the top-level table.\n"
            "\tPass a capability to the new page table, and not for example, the IPC buffer frame vaddr.\n")

        /* TODO 5: then map the frame in */
        /* hint 1: use seL4_ARCH_Page_Map() as above
         * hint 2: for the rights, use seL4_AllRights
         * hint 3: for VM attributes use seL4_ARCH_Default_VMAttributes
         */
		 error = seL4_X86_PageTable_Map(ipc_frame_object.cptr, seL4_X86_PageDirectory pd, ipc_buffer_vaddr, seL4_ARCH_Default_VMAttributes);

        ZF_LOGF_IFERR(error, "Failed again to map the IPC buffer frame into the VSpace.\n"
			"\t(It's not supposed to fail.)\n"
            "\tPass a capability to the IPC buffer's physical frame.\n"
            "\tRevisit the first seL4_ARCH_Page_Map call above and double-check your arguments.\n");
    }

    /* set the IPC buffer's virtual address in a field of the IPC buffer */
    seL4_IPCBuffer *ipcbuf = (seL4_IPCBuffer*)ipc_buffer_vaddr;
    ipcbuf->userData = ipc_buffer_vaddr;

    /* TODO 6: create an endpoint */
    /* hint: vka_alloc_endpoint()
     * int vka_alloc_endpoint(vka_t *vka, vka_object_t *result)
     * @param vka Pointer to vka interface.
     * @param result Structure for the Endpoint object.  This gets initialised.
     * @return 0 on success
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_6:
     */
    error = vka_alloc_endpoint(vka, ep_object);

    ZF_LOGF_IFERR(error, "Failed to allocate new endpoint object.\n");

    /* TODO 7: make a badged copy of it in our cspace. This copy will be used to send
     * an IPC message to the original cap */
    /* hint 1: vka_mint_object()
     * int vka_mint_object(vka_t *vka, vka_object_t *object, cspacepath_t *result, seL4_CapRights rights, seL4_CapData_t badge)
     * @param[in] vka The allocator for the cspace.
     * @param[in] object Target object for cap minting.
     * @param[out] result Allocated cspacepath.
     * @param[in] rights The rights for the minted cap.
     * @param[in] badge The badge for the minted cap.
     * @return 0 on success
     *
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_7:
     *
     * hint 2: for the rights, use seL4_AllRights
     * hint 3: for the badge use seL4_CapData_Badge_new()
     * seL4_CapData_t CONST seL4_CapData_Badge_new(seL4_Uint32 Badge)
     * @param[in] Badge The badge number to use
     * @return A CapData structure containing the desired badge info
     *
     * seL4_CapData_t is generated during build.
     * The type definition and generated field access functions are defined in a generated file:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_7:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 4: for the badge use EP_BADGE
     */
    seL4_CapData_t badge = seL4_CapData_Badge_new(EP_BADGE);
    cspacepath_t cspacepath;

    error = vka_mint_object(vka, ep_object, cspacepath, seL4_AllRights, seL4_CapData_t badge);

    ZF_LOGF_IFERR(error, "Failed to mint new badged copy of IPC endpoint.\n"
        "\tseL4_Mint is the backend for vka_mint_object.\n"
        "\tseL4_Mint is simply being used here to create a badged copy of the same IPC endpoint.\n"
        "\tThink of a badge in this case as an IPC context cookie.\n");

    /* initialise the new TCB */
    error = seL4_TCB_Configure(tcb_object.cptr, seL4_CapNull, seL4_MaxPrio,
        cspace_cap, seL4_NilData, pd_cap, seL4_NilData,
        ipc_buffer_vaddr, ipc_frame_object.cptr);/*TODO: This is different from hello-2*/
    ZF_LOGF_IFERR(error, "Failed to configure the new TCB object.\n"
        "\tWe're running the new thread with the root thread's CSpace.\n"
        "\tWe're running the new thread in the root thread's VSpace.\n");

    /* give the new thread a name */
    name_thread(tcb_object.cptr, "hello-3: thread_2");

    /* set start up registers for the new thread */
    seL4_UserContext regs = {0};
    size_t regs_size = sizeof(seL4_UserContext) / sizeof(z; seL4_Word);

    /* set instruction pointer where the thread shoud start running */
    sel4utils_set_instruction_pointer(&regs, (seL4_Word)thread_2);

    /* check that stack is aligned correctly */
    const int stack_alignment_requirement = sizeof(seL4_Word) * 2;
    uintptr_t thread_2_stack_top = (uintptr_t)thread_2_stack + sizeof(thread_2_stack);
    ZF_LOGF_IF(thread_2_stack_top % (stack_alignment_requirement) != 0,
        "Stack top isn't aligned correctly to a %dB boundary.\n"
        "\tDouble check to ensure you're not trampling, and if sure, ask a \n"
        "\tkernel engineer to review the tutorial code.",
        stack_alignment_requirement);

    /* set stack pointer for the new thread. remember the stack grows down */
    sel4utils_set_stack_pointer(&regs, thread_2_stack_top);

    /* set the gs register for thread local storage */
    regs.gs = IPCBUF_GDT_SELECTOR;

    /* actually write the TCB registers. */
    error = seL4_TCB_WriteRegisters(tcb_object.cptr, 0, 0, regs_size, &regs);
    ZF_LOGF_IFERR(error, "Failed to write the new thread's register set.\n"
        "\tDid you write the correct number of registers? See arg4.\n");

    /* start the new thread running */
    error = seL4_TCB_Resume(tcb_object.cptr);
    ZF_LOGF_IFERR(error, "Failed to start new thread.\n");

    /* we are done, say hello */
    printf("main: hello world\n");

    /*
     * now send a message to the new thread, and wait for a reply
     */

    seL4_Word msg;
    seL4_MessageInfo_t tag;

    /* TODO 8: set the data to send. We send it in the first message register */
    /* hint 1: seL4_MessageInfo_new()
     * seL4_MessageInfo_t CONST seL4_MessageInfo_new(seL4_Uint32 label, seL4_Uint32 capsUnwrapped, seL4_Uint32 extraCaps, seL4_Uint32 length)
     * @param label The value of the label field
     * @param capsUnwrapped The value of the capsUnwrapped field
     * @param extraCaps The value of the extraCaps field
     * @param length The number of message registers to send
     * @return The seL4_MessageInfo_t containing the given values.
     *
     * seL4_MessageInfo_new() is generated during build. It can be found in:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_8:
     *
     * hint 2: use 0 for the first 3 fields.
     * hint 3: send only 1 message register of data
     *
     * hint 4: seL4_SetMR()
     * void seL4_SetMR(int i, seL4_Word mr)
     * @param i The message register to write
     * @param mr The value of the message register
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_8:
     * You can find out more about message registers in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 5: send MSG_DATA
     */

    /* TODO 9: send and wait for a reply. */
    /* hint: seL4_Call()
     * seL4_MessageInfo_t seL4_Call(seL4_CPtr dest, seL4_MessageInfo_t msgInfo)
     * @param dest The capability to be invoked.
     * @param msgInfo The messageinfo structure for the IPC.  This specifies information about the message to send (such as the number of message registers to send).
     * @return A seL4_MessageInfo_t structure.  This is information about the repy message.
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_9:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     *
     * hint 2: seL4_MessageInfo_t is generated during build.
     * The type definition and generated field access functions are defined in a generated file:
     * build/x86/pc99/libsel4/include/sel4/types_gen.h
     * It is generated from the following definition:
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_9:
     * You can find out more about it in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */

    /* TODO 10: get the reply message */
    /* hint: seL4_GetMR()
     * seL4_Word seL4_GetMR(int i)
     * @param i The message register to retreive
     * @return The message register value
     * Link to source: https://wiki.sel4.systems/seL4%20Tutorial%203#TODO_10:
     * You can find out more about message registers in the API manual: http://sel4.systems/Info/Docs/seL4-manual-3.0.0.pdf
     */

    /* check that we got the expected repy */
    ZF_LOGF_IF(seL4_MessageInfo_get_length(tag) != 1,
        "Response data from thread_2 was not the length expected.\n"
        "\tHow many registers did you set with seL4_SetMR within thread_2?\n");
    ZF_LOGF_IF(msg != ~MSG_DATA,
        "Response data from thread_2's content was not what was expected.\n");

    printf("main: got a reply: %#x\n", msg);

    return 0;
}
