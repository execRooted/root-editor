bits 16
org 0x7E00

start:
    
    mov ax, 0x0003
    int 0x10
    
    
    mov ax, 0x0600
    mov bh, 0x07
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10
    
    
    mov ah, 0x02
    mov bh, 0x00
    mov dx, 0x0000
    int 0x10
    
    
    call init_filesystem
    
    
    mov si, welcome_msg
    call print_string
    
    
    jmp shell


init_filesystem:
    
    mov di, file_entries
    mov si, file_boot_bin
    call copy_string
    add di, 32
    mov dword [di], 512    
    add di, 4
    mov dword [di], 0x7C00 
    
    
    mov di, file_entries + 64
    mov si, file_kernel_bin
    call copy_string
    add di, 32
    mov dword [di], 5120   
    add di, 4
    mov dword [di], 0x7E00 
    
    ret


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


print_newline:
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    ret


copy_string:
    .copy_loop:
    lodsb
    stosb
    cmp al, 0
    jne .copy_loop
    ret


trim_string:
    push si
    mov di, si
.find_end:
    cmp byte [di], 0
    je .found_end
    inc di
    jmp .find_end
.found_end:
    dec di
.trim_loop:
    cmp di, si
    jl .done_trim
    cmp byte [di], ' '
    jne .done_trim
    mov byte [di], 0
    dec di
    jmp .trim_loop
.done_trim:
    pop si
    ret


shell:
    call print_newline
    mov si, prompt
    call print_string
    
    
    mov di, input_buffer
    mov cx, 0
.read_loop:
    mov ah, 0x00
    int 0x16
    
    
    cmp al, 0x0D
    je .process_input
    
    
    cmp al, 0x08
    je .backspace
    
    
    cmp cx, 63
    jge .read_loop
    
    
    mov ah, 0x0E
    int 0x10
    
    
    stosb
    inc cx
    jmp .read_loop

.backspace:
    cmp cx, 0
    je .read_loop
    
    
    mov ah, 0x0E
    mov al, 0x08
    int 0x10
    mov al, ' '
    int 0x10
    mov al, 0x08
    int 0x10
    
    
    dec di
    mov byte [di], 0
    dec cx
    jmp .read_loop

.process_input:
    
    mov byte [di], 0
    
    
    call print_newline
    
    
    mov si, input_buffer
    call execute_command
    
    
    mov di, input_buffer
    mov cx, 64
.clear_loop:
    mov byte [di], 0
    inc di
    loop .clear_loop
    
    jmp shell


execute_command:
    
    cmp byte [si], 0
    je .empty_input
    
    
    mov di, cmd_help
    call compare_string
    je .show_help
    
    
    mov di, cmd_list
    call compare_string
    je .show_list
    
    
    mov di, cmd_make
    call compare_string
    je .make_file
    
    
    mov di, cmd_delete
    call compare_string
    je .delete_file
    
    
    mov di, cmd_clear
    call compare_string
    je .clear_screen

    
    mov di, cmd_shutdown
    call compare_string
    je .shutdown
    mov di, cmd_echo
    call compare_string
    je .echo_command

    mov di, cmd_show
    call compare_string
    je .show_file

    mov di, cmd_edit
    call compare_string
    je .edit_file

    mov si, unknown_msg
    call print_string
    ret

.empty_input:
    ret

.show_help:
    mov si, help_msg
    call print_string
    ret

.show_list:
    call list_files
    ret

.make_file:

    add si, 5
    cmp byte [si], 0
    je .make_no_name

    call trim_string

    call create_file
    ret

.make_no_name:
    mov si, make_no_name_msg
    call print_string
    ret

.delete_file:
    
    add si, 7
    cmp byte [si], 0
    je .delete_no_name
    
    
    call delete_file
    ret

.delete_no_name:
    mov si, delete_no_name_msg
    call print_string
    ret

.clear_screen:
    
    mov ax, 0x0600
    mov bh, 0x07
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    
    mov ah, 0x02
    mov bh, 0x00
    mov dx, 0x0000
    int 0x10
    ret


.shutdown:
    mov si, shutdown_msg
    call print_string
    
    hlt
    
    jmp $


.echo_command:
    add si, 5
    call print_string
    ret

.show_file:
    add si, 5
    cmp byte [si], 0
    je .show_no_name
    call trim_string
    call show_file
    ret

.show_no_name:
    mov si, show_no_name_msg
    call print_string
    ret

.edit_file:
    add si, 5
    cmp byte [si], 0
    je .edit_no_name
    call trim_string
    mov di, si
.find_space:
    cmp byte [di], ' '
    je .found_space
    cmp byte [di], 0
    je .edit_no_content
    inc di
    jmp .find_space
.found_space:
    mov byte [di], 0
    inc di
    call edit_file
    ret

.edit_no_name:
    mov si, edit_no_name_msg
    call print_string
    ret

.edit_no_content:
    mov si, edit_no_content_msg
    call print_string
    ret


list_files:
    mov si, list_header
    call print_string
    call print_newline
    
    mov cx, 0
    mov di, file_entries
.list_loop:
    cmp byte [di], 0
    je .list_done
    
    
    mov si, di
    call print_string
    
    
    mov al, 9
    mov ah, 0x0E
    int 0x10
    
    
    add di, 32
    mov eax, [di]
    call print_number
    
    
    mov si, bytes_msg
    call print_string
    
    call print_newline
     
    add di, 32
    inc cx
    cmp cx, 16
    jge .list_done
    jmp .list_loop

.list_done:
    call print_newline
    mov si, list_footer
    call print_string
    ret




create_file:
    
    mov cx, 0
    mov di, file_entries
.find_empty:
    cmp byte [di], 0
    je .found_empty
    
    add di, 64
    inc cx
    cmp cx, 16
    jge .no_space
    
    jmp .find_empty

.found_empty:

    push di
    call copy_string
    pop di

    ; trim the stored filename
    mov si, di
    call trim_string

    add di, 32
    mov dword [di], 0
    add di, 4
    mov ax, [free_mem]
    mov [di], ax
    add word [free_mem], 0x100
    
    mov si, file_created_msg
    call print_string
    ret

.no_space:
    mov si, no_space_msg
    call print_string
    ret


delete_file:
    mov cx, 0
    mov di, file_entries
.find_file:
    push si
    push di
    call compare_string
    pop di
    pop si
    je .found_file
    
    add di, 64
    inc cx
    cmp cx, 16
    jge .file_not_found
    
    jmp .find_file

.found_file:
    
    mov cx, 64
.clear_entry:
    mov byte [di], 0
    inc di
    loop .clear_entry
    
    mov si, file_deleted_msg
    call print_string
    ret

.file_not_found:
    mov si, file_not_found_msg
    call print_string
    ret


show_file:
    mov cx, 0
    mov di, file_entries
.find_file:
    push si
    push di
    call compare_string
    pop di
    pop si
    je .found_file
    add di, 64
    inc cx
    cmp cx, 16
    jge .file_not_found
    jmp .find_file
.found_file:
    mov bx, di
    mov di, [bx+36]
    mov cx, [bx+32]
    cmp cx, 0
    je .empty
.print_loop:
    mov al, [di]
    cmp al, 0
    je .done_print
    mov ah, 0x0E
    int 0x10
    inc di
    dec cx
    jnz .print_loop
.done_print:
    call print_newline
    ret
.empty:
    mov si, empty_file_msg
    call print_string
    ret
.file_not_found:
    mov si, file_not_found_msg
    call print_string
    ret


edit_file:
    mov cx, 0
    mov bx, file_entries
.find_file:
    push si
    push bx
    call compare_string
    pop bx
    pop si
    je .found_file
    add bx, 64
    inc cx
    cmp cx, 16
    jge .file_not_found
    jmp .find_file
.found_file:
    push bx
    mov si, di
    mov di, [bx+36]
    call copy_string
    pop bx
    mov ax, di
    sub ax, [bx+36]
    dec ax
    mov [bx+32], ax
    mov si, file_edited_msg
    call print_string
    ret
.file_not_found:
    mov si, file_not_found_msg
    call print_string
    ret


print_number:
    pusha
    mov cx, 10
    mov bx, 0
    mov di, number_buffer + 10
    mov byte [di], 0
    dec di

.convert_loop:
    xor dx, dx
    div cx
    add dl, '0'
    mov [di], dl
    dec di
    test ax, ax
    jnz .convert_loop

    mov si, di
    inc si
    call print_string
    popa
    ret


compare_string:
    push si
    push di
.compare_loop:
    mov al, [si]
    mov bl, [di]
    
    
    cmp al, 0
    je .check_di_end
    cmp bl, 0
    je .not_equal
    
    
    cmp al, 'A'
    jl .compare_chars
    cmp al, 'Z'
    jg .compare_chars
    add al, 32
    
.compare_chars:
    cmp bl, 'A'
    jl .do_compare
    cmp bl, 'Z'
    jg .do_compare
    add bl, 32

.do_compare:
    cmp al, bl
    jne .not_equal
    
    inc si
    inc di
    jmp .compare_loop

.check_di_end:
    cmp byte [di], 0
    jne .not_equal

.equal:
    pop di
    pop si
    mov ax, 1
    ret

.not_equal:
    pop di
    pop si
    mov ax, 0
    ret





welcome_msg db "root_OS v2.0 by execRooted - Type 'help' for commands", 0
prompt db "shell> ", 0


cmd_help db "help", 0
cmd_list db "list", 0
cmd_make db "make", 0
cmd_delete db "delete", 0
cmd_clear db "clear", 0
cmd_shutdown db "shutdown", 0
cmd_echo db "echo", 0
cmd_show db "show", 0
cmd_edit db "edit", 0


file_boot_bin db "boot.bin", 0
file_kernel_bin db "kernel.bin", 0


help_msg db "Available commands:", 0x0D, 0x0A
          db "  help    - Show this help", 0x0D, 0x0A
          db "  list    - List files", 0x0D, 0x0A
          db "  make    - Create a file (make filename)", 0x0D, 0x0A
          db "  delete  - Delete a file (delete filename)", 0x0D, 0x0A
          db "  clear   - Clear screen", 0x0D, 0x0A
          db "  echo    - Print text", 0x0D, 0x0A
          db "  show    - Display file contents (show filename)", 0x0D, 0x0A
          db "  edit    - Edit file content (edit filename content)", 0x0D, 0x0A
          db "  shutdown- Shutdown the system", 0x0D, 0x0A, 0

list_header db "Files in OS directory:", 0
list_footer db "--- End of file list ---", 0
bytes_msg db " bytes", 0

file_created_msg db "File created successfully", 0x0D, 0x0A, 0
file_deleted_msg db "File deleted successfully", 0x0D, 0x0A, 0
file_not_found_msg db "File not found", 0x0D, 0x0A, 0
no_space_msg db "No space for more files", 0x0D, 0x0A, 0
make_no_name_msg db "Usage: make filename", 0x0D, 0x0A, 0
delete_no_name_msg db "Usage: delete filename", 0x0D, 0x0A, 0
unknown_msg db "Unknown command. Type 'help' for available commands.", 0x0D, 0x0A, 0

empty_file_msg db "File is empty", 0x0D, 0x0A, 0
shutdown_msg db "Shutting down root_OS by execRooted...", 0x0D, 0x0A, 0


show_no_name_msg db "Usage: show filename", 0x0D, 0x0A, 0
edit_no_name_msg db "Usage: edit filename content", 0x0D, 0x0A, 0
edit_no_content_msg db "No content provided", 0x0D, 0x0A, 0
file_edited_msg db "File edited successfully", 0x0D, 0x0A, 0


file_entries times 1024 db 0


input_buffer times 64 db 0


number_buffer times 16 db 0
free_mem dw 0xA000



times 5120-($-$$) db 0