        title   asmsubs.asm

;*********************************************************************/
;*                                                                   */
;*  Windows Cardfile                                                 */
;*  (c) Copyright Microsoft Corp. 1985,1986 - All Rights Reserved    */
;*                                                                   */
;*********************************************************************/
; assembler helper routines

.xlist
include cmacros.inc
.list

createSeg   _NRES1,NONRES1,byte,public,CODE

sBegin  DATA
sEnd    DATA

sBegin  NONRES1

assumes CS,NONRES1
assumes DS,DATA

cProc   RepMov,<PUBLIC,NEAR>,<di,si>
        parmD   lpDest
        parmD   lpSrc
        parmW   cnt
cBegin
        push    ds
        cld
        les     di,lpDest
        lds     si,lpSrc
        mov     cx,cnt
        repne   movsb
        pop     ds
cEnd

cProc   RepMovUp,<PUBLIC,NEAR>,<di,si>
        parmD   lpDest
        parmD   lpSrc
        parmW   cnt
cBegin
        push    ds
        std
        les     di,lpDest
        lds     si,lpSrc
        mov     cx,cnt
        add     si,cx
        add     di,cx
        dec     si
        dec     di
        repne   movsb
        cld
        pop     ds
cEnd

sEnd    NONRES1

end
