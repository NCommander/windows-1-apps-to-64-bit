        title   indos.asm

;****************************************************************/
;*                                                              */
;*  Windows Cardfile - Written by Mark Cliggett                 */
;*  (c) Copyright Microsoft Corp. 1985 - All Rights Reserved    */
;*                                                              */
;****************************************************************/

.xlist
include cmacros.inc

.list

createSeg   _FILE,nrfile,byte,public,CODE

sBegin  DATA
globalD resZero,0
sEnd    DATA

sBegin  NRFILE

assumes CS,NRFILE
assumes DS,DATA

cProc   Frename,<PUBLIC,NEAR>,<di>
    parmW src
    parmW dst
cBegin
        mov     dx,src
        push    ds
        pop     es
        mov     di,dst
        mov     ah,56h
        int     21h
        jc      renexit
        xor     ax,ax
renexit:
cEnd

;
; Fdelete (pch) - delete a file
;   returns 0 or -1
;

cProc   Fdelete,<PUBLIC,FAR>
    parmW src
cBegin
        mov     dx,src
        mov     ah,41h
        int     21h
        jc      rmexit
        xor     ax,ax
rmexit:
cEnd

cProc mylread,<PUBLIC,NEAR>
    parmW fh
    parmD lpbuf
    parmW count
cBegin
    push    ds
    mov     bx,fh
    lds     dx,lpbuf
    mov     ah,3fh
    mov     cx,count
    int     21h
    jnc     lreaddone
    xor     ax,ax
lreaddone:
    pop     ds
cEnd

cProc myread,<PUBLIC,NEAR>
    parmW fh
    parmW buf
    parmW count
cBegin
    mov     bx,fh
    mov     dx,buf
    mov     ah,3fh
    mov     cx,count
    int     21h
    jnc     readdone
    xor     ax,ax
readdone:
cEnd

cProc mylwrite,<PUBLIC,NEAR>
    parmW   fh
    parmD   lpbuf
    parmW   count
cBegin
    push    ds
    mov     bx,fh
    lds     dx,lpbuf
    mov     ah,40h
    mov     cx,count
    int     21h
    jnc     lwritedone
    xor     ax,ax
lwritedone:
    pop     ds
cEnd

cProc mywrite,<PUBLIC,NEAR>
    parmW   fh
    parmW   buf
    parmW   count
cBegin
    mov     bx,fh
    mov     dx,buf
    mov     ah,40h
    mov     cx,count
    int     21h
    jnc     writedone
    xor     ax,ax
writedone:

cEnd

cProc   MyOpen,<PUBLIC,FAR>
        parmD   lpFilename
        parmB   mode
cBegin
        push    ds
        lds     dx,lpFilename
        mov     al,mode
        mov     ah,3Dh
        int     21h
        pop     ds
        jnc     MyOpen_ok
        mov     ax,-1
MyOpen_ok:
cEnd


cProc   MyClose,<PUBLIC,FAR>
        parmW   fd
cBegin
        mov     bx,fd
        mov     ah,3Eh              ; DOS file close function
        int     21h
        mov     ax,-1
        jc      MyClose_end
        inc     ax
MyClose_end:
cEnd

cProc   MyLseek,<PUBLIC,FAR>
    parmW   fh
    parmD   fileOffset
    parmW   mode
cBegin
        mov     dx,off_fileOffset
        mov     cx,seg_fileOffset
        mov     ax,mode
        mov     bx,fh
        mov     ah,42h
        int     21h
        jnc     MyLseek_ok
        mov     ax,-1
        cwd                         ; must return a long
MyLseek_ok:
cEnd

;
; MyStrlen
;

cProc   Mylstrlen,<PUBLIC,FAR>
        parmD   lpStr
cBegin
        push    di
        les     di,lpStr
        cld
        xor     ax,ax                   ; get zero in ax
        mov     cx,-1                   ; at most 64 k to move
        repnz   scasb                   ; look for end
        mov     ax,cx
        neg     ax
        dec     ax
        dec     ax
        pop     di
cEnd

;
;MyStrcpy:
;
cProc   Mylstrcpy,<PUBLIC,FAR>
        parmD   lpDst
        parmD   lpSrc
cBegin
        push    si
        push    di
        push    ds


        lds     si,lpSrc
        les     di,lpDst
        cld
Mycp1:  lodsb
        stosb
        or      al,al
        jnz     Mycp1
        mov     ax,di                   ; point at last byte copied
        dec     ax
        mov     dx,es                   ; and segment
        pop     ds
        pop     di
        pop     si
cEnd


;
;MyStrcat:
;
cProc   Mylstrcat,<PUBLIC,FAR>
    parmD   lpDst
    parmD   lpSrc
cBegin
        push    si
        push    di
        push    ds

        lds     si,lpSrc
        les     di,lpDst
        cld
        xor     ax,ax                   ; get zero in ax
        mov     cx,-1                   ; at most 64 k to look
        repnz   scasb                   ; look for end
        dec     di                      ; Point at null byte
Myct1:  lodsb
        stosb
        or      al,al
        jnz     Myct1
        mov     ax,di                   ; point at last byte copied
        dec     ax
        mov     dx,es                   ; and segment
        pop     ds
        pop     di
        pop     si
cEnd


;
;MyStrcmp:
;
cProc   Mylstrcmp,<PUBLIC,FAR>
        parmD   lps1
        parmD   lps2
cBegin
        push    si
        push    di
        push    ds

        lds     si,lps1
        les     di,lps2
        cld
        xor     ax,ax                       ; clear out ah
Mycmploop:
        lodsb
        mov     bl,es:[di]
        inc     di
        cmp     bl,al
        jnz     MyNeq
        or      al,al
        jnz     Mycmploop
        xor     ax,ax                        ; equal
        jmp     short MSEnd
MyNeq:
        mov     ax,1                         ; not equal
MSEnd:
        pop     ds
        pop     di
        pop     si
cEnd

sEnd    NRFILE

end
