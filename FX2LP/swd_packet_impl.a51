        NAME    ?SWD_PACKET

OED     DATA    0B5H
IOD     DATA    0B0H
SWDIO   BIT     IOD.0
SWDCL   BIT     IOD.1

?PR?_Swd_SendByte SEGMENT CODE
        PUBLIC  _Swd_SendByte
		PUBLIC  Swd_SendByte
        RSEG    ?PR?_Swd_SendByte
		USING   0
_Swd_SendByte:
Swd_SendByte:
        CLR     A
        MOV     R6,A
        MOV     A,R7
?LOOP1:
        CLR     SWDCL
        RRC     A
        MOV     SWDIO,C
        SETB    SWDCL
        INC     R6
        CJNE    R6,#08H,?LOOP1
        RET     

?PR?_Swd_ReceiveByte SEGMENT CODE
        PUBLIC  _Swd_ReceiveByte
		PUBLIC  Swd_ReceiveByte
        RSEG    ?PR?_Swd_ReceiveByte
		USING   0
_Swd_ReceiveByte:
Swd_ReceiveByte:
        CLR     A
        MOV     R6,A
?LOOP2:
        CLR     SWDCL
        MOV     C,SWDIO
        SETB    SWDCL
        RRC     A
        INC     R6
        CJNE    R6,#08H,?LOOP2
        MOV     R7, A
        RET     

?PR?_Swd_FirstTurnAroundPhase SEGMENT CODE
        PUBLIC  _Swd_FirstTurnAroundPhase
		PUBLIC  Swd_FirstTurnAroundPhase
        RSEG    ?PR?_Swd_FirstTurnAroundPhase
		USING   0
_Swd_FirstTurnAroundPhase:
Swd_FirstTurnAroundPhase:
        MOV     A, OED
        ANL     A, #0FEH
        MOV     OED, A        ; SWDIO is now input
        CLR     SWDCL
        SETB    SWDCL
        RET

?PR?_Swd_SecondTurnAroundPhase SEGMENT CODE
        PUBLIC  _Swd_SecondTurnAroundPhase
		PUBLIC  Swd_SecondTurnAroundPhase
        RSEG    ?PR?_Swd_SecondTurnAroundPhase
		USING   0
_Swd_SecondTurnAroundPhase:
Swd_SecondTurnAroundPhase:
        MOV     A, OED
        ORL     A, #1
        MOV     OED, A        ; SWDIO is now output
        CLR     SWDCL
        SETB    SWDCL
        RET

?PR?_Swd_GetAckSegment SEGMENT CODE
        PUBLIC  _Swd_GetAckSegment
		PUBLIC  Swd_GetAckSegment
        RSEG    ?PR?_Swd_GetAckSegment
		USING   0
_Swd_GetAckSegment:
Swd_GetAckSegment:
        CLR     A
        CLR     SWDCL
        MOV     C,SWDIO
        SETB    SWDCL
        MOV     ACC.0, C
        CLR     SWDCL
        MOV     C,SWDIO
        SETB    SWDCL
        MOV     ACC.1, C
        CLR     SWDCL
        MOV     C,SWDIO
        SETB    SWDCL
        MOV     ACC.2, C
		MOV     R7, A
        RET

?PR?_Swd_LineReset SEGMENT CODE
        PUBLIC  _Swd_LineReset
		PUBLIC  Swd_LineReset
        RSEG    ?PR?_Swd_LineReset
		USING   0
_Swd_LineReset:
Swd_LineReset:
        SETB    SWDIO
		MOV     A, #51
?LOOP3:
        CLR     SWDCL
		SETB    SWDCL
		DJNZ    ACC, ?LOOP3
        CLR     SWDCL
        CLR     SWDIO
		SETB    SWDCL
        RET
        END
