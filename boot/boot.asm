bits 16
org 0x7C00

start:
    
    cli
    
    
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    
    mov ax, 0x0000
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    
    mov ax, 0x0003
    int 0x10
    
    
    mov si, loading_msg
    call print_string
    
    
    mov ah, 0x02        
    mov al, 10          
    mov ch, 0           
    mov cl, 2           
    mov dh, 0           
    mov dl, 0x80        
    mov bx, 0x7E00      
    int 0x13
    
    jc disk_error       
    
    
    jmp 0x7E00

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.print_loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .print_loop
.done:
    ret

loading_msg db "Booting root_OS by execRooted...", 0x0D, 0x0A, 0
error_msg db "Disk read error! Press any key to reboot.", 0x0D, 0x0A, 0


times 510-($-$$) db 0
dw 0xAA55