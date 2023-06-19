_buffer_template:
    .int 32 ;(0x00)
    .int 0 ;(0x04)
    .int 0x00038041 ;(0x08)
    .int 8 ;(0x0c)
    .int 0 ;(0x10)
    .int 130 ;(0x14)
    .int 1 ;(0x18)
    .int 0 ;(0x1c)

turn_on_status:
    .int 1
  
turn_off_status:
    .int 0

pin:
    .int 130

buffer_address:
	.int 0x80000

led_status_address:
    .int 0x80018

pin_address:
	.int 0x80014

read_register_address:
	.int 0x3f00b880

write_register_address:
	.int 0x3f00b8a0

status_register_address:
	.int 0x3f00b8b8

ldr w0, pin
ldr w1, turn_on_status
ldr w2, turn_off_status
ldr w3, buffer_address
ldr w4, led_status_address
ldr w5, pin_address
ldr w6, read_register_address
ldr w7, write_register_address
ldr w8, status_register_address

check_write_full_flag_on:
    ldr w20, [w8]
    orr w20, wzr, w20, ror #31
    cmp w20, #1
    b.eq check_write_full_flag_on

load_to_write_register:
    str w1, [w4]
    str w0, [w5]
    orr w20, wzr, w3, lsl #4
    add w20, w20, #8
    str w20, [w7]
    movz w15 #0xFFFFFFFF

write_delay:
    subs w15, w15, #1
    b.ne write_delay

check_read_full_flag_on:
    ldr w20, [w8]
    orr w20, wzr, w20, ror #30
    and w20, w20, #1
    cmp w20, #1
    b.eq check_read_full_flag_on

read_from_read_register:
    ldr w20, [w6]

read_delay:
    subs w15, w15, #1
    b.ne read_delay

and x0, x0, x0
b check_write_full_flag_on
