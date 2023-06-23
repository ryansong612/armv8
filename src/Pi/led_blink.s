ldr w0, buffer_address
ldr w1, buffer_1
ldr w2, buffer_2
ldr w3, buffer_3
ldr w4, buffer_4
ldr w5, buffer_5
ldr w6, buffer_6
ldr w7, buffer_7_on
ldr w8, buffer_7_off
ldr w9, buffer_8
ldr w10, read_register_address
ldr w11, write_register_address
ldr w12, status_register_address
ldr w13, buffer_address_with_channel


check_write_full_flag_on:
    ldr w20, [w12]
    orr w20, wzr, w20, lsr #31
    cmp w20, #1
    b.eq check_write_full_flag_on

load_to_write_register_on:
    str w1, [w0]
    str w2, [w0, #4]
    str w3, [w0, #8]
    str w4, [w0, #12]
    str w5, [w0, #16]
    str w6, [w0, #20]
    str w7, [w0, #24]
    str w9, [w0, #28]
    str w13, [w11]
    ldr w15, delay

on_delay:
   subs w15, w15, #1
   b.ne on_delay

read_from_read_register_on:
    ldr w20, [w10]

check_write_full_flag_off:
    ldr w20, [w12]
    orr w20, wzr, w20, lsr #31
    cmp w20, #1
    b.eq check_write_full_flag_off

load_to_write_register_off:
    str w1, [w0]
    str w2, [w0, #4]
    str w3, [w0, #8]
    str w4, [w0, #12]
    str w5, [w0, #16]
    str w6, [w0, #20]
    str w8, [w0, #24]
    str w9, [w0, #28]
    str w13, [w11]

    ldr w15, delay

off_delay:
    subs w15, w15, #1
    b.ne off_delay

read_from_read_register_off:
    ldr w20, [w10]

b check_write_full_flag_on


buffer_1:
    .int 32

buffer_2:
    .int 0

buffer_3:
    .int 0x38041

buffer_4:
    .int 8

buffer_5:
    .int 0

buffer_6:
    .int 130

buffer_7_on:
    .int 1

buffer_7_off:
    .int 0

buffer_8:
    .int 0

buffer_address:
    .int 0x81000

buffer_address_with_channel:
    .int 0x81008

read_register_address:
    .int 0x3f00b880

write_register_address:
    .int 0x3f00b8a0

status_register_address:
    .int 0x3f00b8b8

delay:
    .int 1000000
