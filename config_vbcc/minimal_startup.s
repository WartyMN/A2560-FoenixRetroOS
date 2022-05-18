; from PJW on Discord, 2022-05-16
;  I think this would be about the minimum... suitable if you don't care what you go back to after quitting the program:

            xdef ___exit
            xdef ___COLDBOOT

            section "CODE",code

            ;    set stack pointer
___COLDBOOT lea    ___STACK,a7

            ;    clear bss, could be optimized to use .l
            lea    ___BSSSTART,a0
            move.l    #___BSSSIZE,d0
            beq    empty
loop:
            move.b    #0,(a0)+
            subq.l    #1,d0
            bne    loop

empty:
            ;    call __main
            jsr    ___main

            ;    endless loop; can be changed accordingly
___exit:
            bra    ___exit