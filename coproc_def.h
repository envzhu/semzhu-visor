#ifndef _COPROC_H_INCLUDED_
#define _COPROC_H_INCLUDED_

/*
 * Secure Configuration Register
 * SCR_EL3
 */
#define SCR_NS  (1 << 0)  
#define SCR_IRQ (1 << 1)  // Virtual IRQ interrupt
#define SCR_FIQ (1 << 2)  // Virtual FIQ interrupt
#define SCR_EA  (1 << 3)  // Virtual External Abort and SError interrupt
#define SCR_SMD (1 << 7)  // Disable Secure Monitor Call, See reference for more detail 
#define SCR_HCE (1 << 8)  // Enable Hyper Visor Call
#define SCR_SIF (1 << 9)  // Disable instruction in Secure state fetches from Non-secure memory
#define SCR_RW  (1 << 10) // Control register width (AArch32 or AArch64)
#define SCR_ST  (1 << 11) // Enables Secure EL1 access to the CNTPS_*_EL1 registers.
#define SCR_TWI (1 << 12) // Trap "wfi" instruction
#define SCR_TWE (1 << 13) // Trap "wfe" instruction


/* 
 * Hypervisor Configuration Register
 * HCR_EL2
 */
#define HCR_VM  (1 << 0)  // Enable Stage 2 MMU
#define HCR_SWIO (1 << 1) // 

#define HCR_FMO (1 << 3)  // Virtual FIQ interrupt
#define HCR_IMO (1 << 4)  // Virtual IRQ interrupt
#define HCR_AMO (1 << 5)  // Virtual External Abort and SError interrupt
#define HCR_VF  (1 << 6)  // Virtual FIQ is pending
#define HCR_VI  (1 << 7)  // Virtual IRQ is pending
#define HCR_VSE (1 << 8)  // Virtual External Abort and SError is pending
#define HCR_SMD (1 << 7)  // Disable Secure Monitor Call, See reference for more detail 
#define HCR_HCE (1 << 8)  // Enable Hyper Visor Call
#define HCR_SIF (1 << 9)  // Disable instruction in Secure state fetches from Non-secure memory
#define HCR_ST  (1 << 11) // Enables Secure EL1 access to the CNTPS_*_EL1 registers.
#define HCR_TWI (1 << 13) // Trap "WFI" instruction
#define HCR_TWE (1 << 14) // Trap "WFE" instruction
#define HCR_TID0 (1 << 15)// Trap  instruction
#define HCR_TID1 (1 << 16)// Trap  instruction
#define HCR_TID2 (1 << 17)// Trap  instruction
#define HCR_TID3 (1 << 19)// Trap  instruction
#define HCR_TSC (1 << 19) // Trap "SMC" instruction

#define HCR_TGE (1 << 28) // Trap general exceptions
#define HCR_TDZ (1 << 28) // Trap DC ZVA instruction

#define HCR_RW  (1 << 31) // Control register width (AArch32 or AArch64)
#define HCR_CD  (1 << 32) // Disable Stage 2 Data cache
#define HCR_ID  (1 << 33) // Disable Stage 2 Instruction cache

/* 
 * Current Program Status Register in AArch64
 * CPSR, SPSR_ELx
 */
#define CPSR_M_EL0t 0b0000
#define CPSR_M_EL1t 0b0100  // the SP is always SP0.
#define CPSR_M_EL1h 0b0101  // the exception SP is determined by the ELx
#define CPSR_M_EL2t 0b1000  // the SP is always SP0.
#define CPSR_M_EL2h 0b1001  // the exception SP is determined by the ELx

#define CPSR_F  (1 << 6) // Diasble FIQ
#define CPSR_I  (1 << 7) // Disable IRQ
#define CPSR_A  (1 << 8) // DIsable SError (System Error)
#define CPSR_E  (1 << 9) // Endianness execution state bit, 1 : Big-endian operation 

#define CPSR_IL (1 << 20) // Illegal Execution State bit 
#define CPSR_SS (1 << 21) // Software step

#define CPSR_V (1 << 28) // Overflow condition flag
#define CPSR_C (1 << 29) // Carry condition flag
#define CPSR_Z (1 << 30) // Zero condition flag
#define CPSR_N (1 << 31) // Negative condition flag

/*
 * System Control Registers of EL2 and EL3
 * SCTLR_EL2, SCTLR_EL3
 */
#define SCTLR_M (1 << 0)  // Enable MMU
#define SCTLR_A (1 << 1)  // Enable alignment fault checking
#define SCTLR_C (1 << 2)  // Enable data and unified caches
#define SCTLR_SA (1 << 3) // Enables stack alignment check.
#define SCTLR_I (1 << 12) // Instruction caches enabled
#define SCTLR_WXN (1 << 19) // Regions with write permissions are forced XN ??
#define SCTLR_EE  (1 << 25) // Exception endianness is Big endian

/*
 * Stage 1 EL1 MMU control registers are :
 * - MAIR_EL1  : Memory Attribute Indirection Register (EL1) キャッシュポリシーの設定。ここで設定した値のインデックスをページテーブルエントリで指定する。
 * - TCR_EL1   : Translation Control Register (EL1) MMUの設定
 * - TTBR0_EL1 : Translation Table Base Register 0 (EL1) 変換テーブルアドレス
 * - TTBR1_EL1 : Translation Table Base Register 1 (EL1) 変換テーブルアドレス 使わない
 * - SCTLR_EL1 : System Control Register (EL1) システムの設定、MMUの有効化はこのレジスタ
 */


/*
 * System Control Registers of EL2 and EL3
 * SCTLR_EL2, SCTLR_EL3
 */

/*
 * Generic Timer registers
 */
#define CNTxx_CTL_ENABLE   (1 << 0)
#define CNTxx_CTL_IMASK    (1 << 1)
#define CNTxx_CTL_ISTATUS  (1 << 2)

/*
 * Exception class defines
*/
#define EC_UNKNOWN_REASON 0x00
#define EC_TRAP_WFx 0x01 /* WFI or WFE instruction execution */
/* EC 0x02 is not used in ARMv8.0*/
#define EC_TRAP_MCR_MRC_CP15_A32  0x03 /* MCR or MRC access to CP15 that is not reported using EC 0x00 */
#define EC_TRAP_MCRR_MRRC_CP15_A32 0x04 /* MCRR or MRRC access to CP15 that is not reported using EC 0x00 */
#define EC_TRAP_MCR_MRC_CP14_A32  0x05  /* MCR or MRC access to CP14 */
#define EC_TRAP_LDC_STC_CP14_A32  0x06  /* LDC or STC access to CP14 */
#define EC_TRAP_SMID_FPREG_A32    0x07  /* Access to SIMD or floating-point registers, excluding (HCR_EL2.TGE==1) traps */
#define EC_TRAP_VMRS_A32    0x08  /* VMRS access(MCR or MRC access to CP10), from ID group traps, this is not reported using EC 0x07 */
/* EC 0x09 is not used in ARMv8.0 but in ARMv8.3*/
/* EC 0x0A is not used in ARMv8.0*/
/* EC 0x0B is not used in ARMv8.0*/
#define EC_TRAP_MRRC_CP14_A32			  0x0C /* MRRC access to CP14 */
/* EC 0x0D is not used in ARMv8.0*/
#define EC_ILLEGAL_EXECUTE  0x0E  /* Illegal Execution State */
/* EC 0x0F is not used in ARMv8.0*/
#define EC_TRAP_SVC_A32     0x11  /* SVC instruction execution from AArch32 */
#define EC_HVC_A32          0x12  /* HVC instruction execution from AArch32 when SMC is not disabled*/
#define EC_TRAP_SMC_A32     0x13  /* SMC instruction execution from AArch32 when SMC is not disabled */
/* EC 0x14 is not used in ARMv8.0*/
#define EC_TRAP_SVC_A64     0x15  /* SVC instruction execution from AArch64 */
#define EC_HVC_A64          0x16  /* HVC instruction execution from AArch64 when HVC is not disabled */
#define EC_TRAP_SMC_A64     0x17  /* SMC instruction execution from AArch64 when SMC is not disabled*/
#define EC_TRAP_MSR_MRS_SYSTEM  0x18 /*MSR, MRS, or System instruction execution, that is not reported using EC 0x00, 0x01, or 0x07 */
/* EC 0x19 is not used in ARMv8.0 but if SVE is implemented */
/* EC 0x1A ~ 0x1E is not used in ARMv8.0 but in ARMv8.3*/
#define EC_IMPLEMENTATION_DEF_EL3 0x1F /* IMPLEMENTATION DEFINED exception taken to EL3 */
#define EC_INST_ABORT_LOWER_EL    0x20  /* Instruction Abort from a lower Exception level */
#define EC_INST_ABORT_CUR_EL      0x21  /* Instruction Abort taken without a change in Exception level */

#define EC_PC_ALIGN_FAULT         0x22  /* Misaligned PC exception */
/* EC 0x23 is not used in ARMv8.0 */
#define EC_INST_ABORT_LOWER_EL    0x24  /* Data Abort from a lower Exception level */
#define EC_INST_ABORT_CUR_EL      0x25  /* Data Abort taken without a change in Exception level */
#define EC_SP_ALIGN_FAULT         0x26  /* Misaligned Stack Pointer exception, this is taken only from AArch64 */
/* EC 0x27 is not used in ARMv8.0 */
#define EC_FP_EXCEP_A32      0x28   /* Floating-point exception taken from AArch32*/
/* EC 0x29 = 0x2B is not used in ARMv8.0 */
#define EC_FP_EXCEP_A64      0x2C   /* Floating-point exception taken from AArch64*/
/* EC 0x2D = 0x2E is not used in ARMv8.0 */
#define EC_SERROR       0x2F  /* SError interrupt */
#define EC_BREAKPOINT_LOWER_EL    0x30  /* Breakpoint exception from a lower Exception level */
#define EC_BREAKPOINT_CUR_EL      0x31  /* Breakpoint exception taken without a change in Exception level */
#define EC_SOFTWARE_STEP_LOWER_EL 0x32  /* Software Step exception from a lower Exception level */
#define EC_SOFTWARE_STEP_CUR_EL   0x33  /* Software Step exception taken without a change in Exception level */
#define EC_WATCHPOINT_LOWER_EL    0x34  /* Watchpoint exception from a lower Exception level */
#define EC_WATCHPOINT_CUR_EL      0x35  /* Watchpoint xception taken without a change in Exception level */
/* EC 0x36 = 0x37 is not used in ARMv8.0 */
#define EC_BKPT_A32 0x38 /* BKpt instruction execution from AArch32 */
/* EC 0x39 is not used in ARMv8.0 */
#define EC_VECTOR_CATCH_EXCP_A32 0x3A /* Vector Catch exception from AArch32 */
/* EC 0x3b is not used in ARMv8.0 */
#define EC_BRK_A64 0x3C /* BRK instruction execution from AArch64 */



#endif
