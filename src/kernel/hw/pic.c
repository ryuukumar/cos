
// #include <kernel/hw/pic.h>

// // See: https://wiki.osdev.org/8259_PIC
// // TODO: Move to APIC (long-term)

// #define ICW1_ICW4	    0x01		/* Indicates that ICW4 will be present */
// #define ICW1_SINGLE	    0x02		/* Single (cascade) mode */
// #define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
// #define ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
// #define ICW1_INIT	    0x10		/* Initialization - required! */

// #define ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
// #define ICW4_AUTO	    0x02		/* Auto (normal) EOI */
// #define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
// #define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
// #define ICW4_SFNM	    0x10		/* Special fully nested (not) */

// #define CASCADE_IRQ 2

// inline void pic_send_eoi(uint8_t irq) {
//     if (irq >= 8) outb(PIC2_COMMAND, PIC_EOI);
//     outb(PIC1_COMMAND, PIC_EOI);
// }

// /*!
//  * Remap pic to specified offsets.
//  * See: https://wiki.osdev.org/8259_PIC
//  * @param offset1 master PIC vector offset
//  * @param offset2 slave PIC vector offset
//  */
// static void pic_remap(int offset1, int offset2)
// {
// 	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
// 	io_wait();
// 	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
// 	io_wait();
// 	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
// 	io_wait();
// 	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
// 	io_wait();
// 	outb(PIC1_DATA, 1 << CASCADE_IRQ);        // ICW3: tell Master PIC that there is a slave PIC at IRQ2
// 	io_wait();
// 	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
// 	io_wait();
	
// 	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
// 	io_wait();
// 	outb(PIC2_DATA, ICW4_8086);
// 	io_wait();
// }

// void __init_pic__ (void) {
//     pic_remap(0x20, 0x28);
// }