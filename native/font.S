.section .rodata.maple,"a",%progbits
.balign 16
.global maple_start
.hidden maple_start
maple_start:
.incbin "native/Maple-Regular.ttf"
.global maple_end
.hidden maple_end
maple_end:
.section .note.GNU-stack,"",%progbits

.section .rodata.ui,"a",%progbits
.balign 16
.global ui_start
.hidden ui_start
ui_start:
.incbin "native/TernUI-Regular.ttf"
.global ui_end
.hidden ui_end
ui_end:
