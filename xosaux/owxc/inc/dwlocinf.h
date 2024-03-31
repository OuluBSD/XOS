/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


/* LOC OP info */
DW_LOC_OP( fil00,           DW_LOP_NOOP )
DW_LOC_OP( fil01,           DW_LOP_NOOP )
DW_LOC_OP( fil02,           DW_LOP_NOOP )
DW_LOC_OP( addr,            DW_LOP_ADDR )
DW_LOC_OP( fil04,           DW_LOP_NOOP )
DW_LOC_OP( fil05,           DW_LOP_NOOP )
DW_LOC_OP( deref,           DW_LOP_NOOP )
DW_LOC_OP( fil07,           DW_LOP_NOOP )
DW_LOC_OP( const1u,         DW_LOP_OPU1 )
DW_LOC_OP( const1s,         DW_LOP_OPS1 )
DW_LOC_OP( const2u,         DW_LOP_OPU2 )
DW_LOC_OP( const2s,         DW_LOP_OPS2 )
DW_LOC_OP( const4u,         DW_LOP_OPU4 )
DW_LOC_OP( const4s,         DW_LOP_OPS4 )
DW_LOC_OP( const8u,         DW_LOP_OPU8 )
DW_LOC_OP( const8s,         DW_LOP_OPS8 )
DW_LOC_OP( constu,          DW_LOP_U128 )
DW_LOC_OP( consts,          DW_LOP_S128 )
DW_LOC_OP( dup,             DW_LOP_NOOP )
DW_LOC_OP( drop,            DW_LOP_NOOP )
DW_LOC_OP( over,            DW_LOP_NOOP )
DW_LOC_OP( pick,            DW_LOP_OPU1 )
DW_LOC_OP( swap,            DW_LOP_NOOP )
DW_LOC_OP( rot,             DW_LOP_NOOP )
DW_LOC_OP( xderef,          DW_LOP_NOOP )
DW_LOC_OP( abs,             DW_LOP_STK1 )
DW_LOC_OP( and,             DW_LOP_STK2 )
DW_LOC_OP( div,             DW_LOP_STK2 )
DW_LOC_OP( minus,           DW_LOP_STK2 )
DW_LOC_OP( mod,             DW_LOP_STK2 )
DW_LOC_OP( mul,             DW_LOP_STK2 )
DW_LOC_OP( neg,             DW_LOP_STK1 )
DW_LOC_OP( not,             DW_LOP_STK1 )
DW_LOC_OP( or,              DW_LOP_STK2 )
DW_LOC_OP( plus,            DW_LOP_STK2 )
DW_LOC_OP( plus_uconst,     DW_LOP_U128 )
DW_LOC_OP( shl,             DW_LOP_STK2 )
DW_LOC_OP( shr,             DW_LOP_STK2 )
DW_LOC_OP( shra,            DW_LOP_STK2 )
DW_LOC_OP( xor,             DW_LOP_STK2 )
DW_LOC_OP( bra,             DW_LOP_OPS2 )
DW_LOC_OP( eq,              DW_LOP_STK2 )
DW_LOC_OP( ge,              DW_LOP_STK2 )
DW_LOC_OP( gt,              DW_LOP_STK2 )
DW_LOC_OP( le,              DW_LOP_STK2 )
DW_LOC_OP( lt,              DW_LOP_STK2 )
DW_LOC_OP( ne,              DW_LOP_STK2 )
DW_LOC_OP( skip,            DW_LOP_OPS2 )
DW_LOC_OP( lit0,            DW_LOP_LIT1 )
DW_LOC_OP( lit1,            DW_LOP_LIT1 )
DW_LOC_OP( lit2,            DW_LOP_LIT1 )
DW_LOC_OP( lit3,            DW_LOP_LIT1 )
DW_LOC_OP( lit4,            DW_LOP_LIT1 )
DW_LOC_OP( lit5,            DW_LOP_LIT1 )
DW_LOC_OP( lit6,            DW_LOP_LIT1 )
DW_LOC_OP( lit7,            DW_LOP_LIT1 )
DW_LOC_OP( lit8,            DW_LOP_LIT1 )
DW_LOC_OP( lit9,            DW_LOP_LIT1 )
DW_LOC_OP( lit10,           DW_LOP_LIT1 )
DW_LOC_OP( lit11,           DW_LOP_LIT1 )
DW_LOC_OP( lit12,           DW_LOP_LIT1 )
DW_LOC_OP( lit13,           DW_LOP_LIT1 )
DW_LOC_OP( lit14,           DW_LOP_LIT1 )
DW_LOC_OP( lit15,           DW_LOP_LIT1 )
DW_LOC_OP( lit16,           DW_LOP_LIT1 )
DW_LOC_OP( lit17,           DW_LOP_LIT1 )
DW_LOC_OP( lit18,           DW_LOP_LIT1 )
DW_LOC_OP( lit19,           DW_LOP_LIT1 )
DW_LOC_OP( lit20,           DW_LOP_LIT1 )
DW_LOC_OP( lit21,           DW_LOP_LIT1 )
DW_LOC_OP( lit22,           DW_LOP_LIT1 )
DW_LOC_OP( lit23,           DW_LOP_LIT1 )
DW_LOC_OP( lit24,           DW_LOP_LIT1 )
DW_LOC_OP( lit25,           DW_LOP_LIT1 )
DW_LOC_OP( lit26,           DW_LOP_LIT1 )
DW_LOC_OP( lit27,           DW_LOP_LIT1 )
DW_LOC_OP( lit28,           DW_LOP_LIT1 )
DW_LOC_OP( lit29,           DW_LOP_LIT1 )
DW_LOC_OP( lit30,           DW_LOP_LIT1 )
DW_LOC_OP( lit31,           DW_LOP_LIT1 )
DW_LOC_OP( reg0,            DW_LOP_REG1 )
DW_LOC_OP( reg1,            DW_LOP_REG1 )
DW_LOC_OP( reg2,            DW_LOP_REG1 )
DW_LOC_OP( reg3,            DW_LOP_REG1 )
DW_LOC_OP( reg4,            DW_LOP_REG1 )
DW_LOC_OP( reg5,            DW_LOP_REG1 )
DW_LOC_OP( reg6,            DW_LOP_REG1 )
DW_LOC_OP( reg7,            DW_LOP_REG1 )
DW_LOC_OP( reg8,            DW_LOP_REG1 )
DW_LOC_OP( reg9,            DW_LOP_REG1 )
DW_LOC_OP( reg10,           DW_LOP_REG1 )
DW_LOC_OP( reg11,           DW_LOP_REG1 )
DW_LOC_OP( reg12,           DW_LOP_REG1 )
DW_LOC_OP( reg13,           DW_LOP_REG1 )
DW_LOC_OP( reg14,           DW_LOP_REG1 )
DW_LOC_OP( reg15,           DW_LOP_REG1 )
DW_LOC_OP( reg16,           DW_LOP_REG1 )
DW_LOC_OP( reg17,           DW_LOP_REG1 )
DW_LOC_OP( reg18,           DW_LOP_REG1 )
DW_LOC_OP( reg19,           DW_LOP_REG1 )
DW_LOC_OP( reg20,           DW_LOP_REG1 )
DW_LOC_OP( reg21,           DW_LOP_REG1 )
DW_LOC_OP( reg22,           DW_LOP_REG1 )
DW_LOC_OP( reg23,           DW_LOP_REG1 )
DW_LOC_OP( reg24,           DW_LOP_REG1 )
DW_LOC_OP( reg25,           DW_LOP_REG1 )
DW_LOC_OP( reg26,           DW_LOP_REG1 )
DW_LOC_OP( reg27,           DW_LOP_REG1 )
DW_LOC_OP( reg28,           DW_LOP_REG1 )
DW_LOC_OP( reg29,           DW_LOP_REG1 )
DW_LOC_OP( reg30,           DW_LOP_REG1 )
DW_LOC_OP( reg31,           DW_LOP_REG1 )
DW_LOC_OP( breg0,           DW_LOP_BRG1 )
DW_LOC_OP( breg1,           DW_LOP_BRG1 )
DW_LOC_OP( breg2,           DW_LOP_BRG1 )
DW_LOC_OP( breg3,           DW_LOP_BRG1 )
DW_LOC_OP( breg4,           DW_LOP_BRG1 )
DW_LOC_OP( breg5,           DW_LOP_BRG1 )
DW_LOC_OP( breg6,           DW_LOP_BRG1 )
DW_LOC_OP( breg7,           DW_LOP_BRG1 )
DW_LOC_OP( breg8,           DW_LOP_BRG1 )
DW_LOC_OP( breg9,           DW_LOP_BRG1 )
DW_LOC_OP( breg10,          DW_LOP_BRG1 )
DW_LOC_OP( breg11,          DW_LOP_BRG1 )
DW_LOC_OP( breg12,          DW_LOP_BRG1 )
DW_LOC_OP( breg13,          DW_LOP_BRG1 )
DW_LOC_OP( breg14,          DW_LOP_BRG1 )
DW_LOC_OP( breg15,          DW_LOP_BRG1 )
DW_LOC_OP( breg16,          DW_LOP_BRG1 )
DW_LOC_OP( breg17,          DW_LOP_BRG1 )
DW_LOC_OP( breg18,          DW_LOP_BRG1 )
DW_LOC_OP( breg19,          DW_LOP_BRG1 )
DW_LOC_OP( breg20,          DW_LOP_BRG1 )
DW_LOC_OP( breg21,          DW_LOP_BRG1 )
DW_LOC_OP( breg22,          DW_LOP_BRG1 )
DW_LOC_OP( breg23,          DW_LOP_BRG1 )
DW_LOC_OP( breg24,          DW_LOP_BRG1 )
DW_LOC_OP( breg25,          DW_LOP_BRG1 )
DW_LOC_OP( breg26,          DW_LOP_BRG1 )
DW_LOC_OP( breg27,          DW_LOP_BRG1 )
DW_LOC_OP( breg28,          DW_LOP_BRG1 )
DW_LOC_OP( breg29,          DW_LOP_BRG1 )
DW_LOC_OP( breg30,          DW_LOP_BRG1 )
DW_LOC_OP( breg31,          DW_LOP_BRG1 )
DW_LOC_OP( regx,            DW_LOP_U128 )
DW_LOC_OP( fbreg,           DW_LOP_S128 )
DW_LOC_OP( bregx,           DW_LOP_U128_S128 )
DW_LOC_OP( piece,           DW_LOP_U128 )
DW_LOC_OP( deref_size,      DW_LOP_OPU1 )
DW_LOC_OP( xderef_size,     DW_LOP_OPU1 )
DW_LOC_OP( nop,             DW_LOP_NOOP )
DW_LOC_OP( lo_user,         DW_LOP_NOOP )
DW_LOC_OP( hi_user,         DW_LOP_NOOP )
