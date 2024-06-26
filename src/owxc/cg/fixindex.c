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
* Description:  Reduce memory references to one per instruction.
*
****************************************************************************/


#include "standard.h"
#include "coderep.h"
#include "opcodes.h"
#include "procdef.h"
#include "addrname.h"
#include "x87.h"


extern  block           *HeadBlock;

extern  bool            IndexOkay(instruction*,name*);
extern  bool            RTLeaveOp2(instruction*);
extern  instruction     *MakeMove(name*,name*,type_class_def);
extern  name            *AllocTemp(type_class_def);
extern  name            *ScaleIndex(name*,name*,type_length,type_class_def,type_length,int,i_flags);
extern  void            FixFPConsts(instruction*);
extern  void            PrefixIns(instruction*,instruction*);
extern  void            SuffixIns(instruction*,instruction*);

static byte NumTab[LAST_OP-FIRST_OP+1] = {
/*****************************************
    Give the number of operand for each opcode. Anything above this
    is probably a segment override tagging along for the ride.
*/
        0,                                      /* OP_NOP*/
        2,                                      /* OP_ADD*/
        2,                                      /* OP_EXT_ADD*/
        2,                                      /* OP_SUB*/
        2,                                      /* OP_EXT_SUB*/
        2,                                      /* OP_MUL*/
        2,                                      /* OP_EXT_MUL*/
        2,                                      /* OP_DIV*/
        2,                                      /* OP_MOD*/
        2,                                      /* OP_AND*/
        2,                                      /* OP_OR*/
        2,                                      /* OP_XOR*/
        2,                                      /* OP_RSHIFT*/
        2,                                      /* OP_LSHIFT*/
        2,                                      /* OP_POW */
        2,                                      /* OP_P5DIV */
        2,                                      /* OP_ATAN2 */
        2,                                      /* OP_FMOD */
        1,                                      /* OP_NEGATE*/
        1,                                      /* OP_COMPLEMENT*/
        1,                                      /* OP_LOG*/
        1,                                      /* OP_COS*/
        1,                                      /* OP_SIN*/
        1,                                      /* OP_TAN*/
        1,                                      /* OP_SQRT*/
        1,                                      /* OP_FABS*/
        1,                                      /* OP_ACOS */
        1,                                      /* OP_ASIN */
        1,                                      /* OP_ATAN */
        1,                                      /* OP_COSH */
        1,                                      /* OP_EXP */
        1,                                      /* OP_LOG10 */
        1,                                      /* OP_SINH */
        1,                                      /* OP_TANH */
        1,                                      /* OP_PTR_TO_NATIVE */
        1,                                      /* OP_PTR_TO_FOREIGN */
        0,                                      /* OP_SLACK_19 */
        1,                                      /* OP_CONVERT*/
        1,                                      /* OP_LA*/
        1,                                      /* OP_CAREFUL_LA*/
        1,                                      /* OP_ROUND*/
        1,                                      /* OP_MOV*/
        3,                                      /* OP_CALL_INDIRECT*/
        1,                                      /* OP_PUSH*/
        0,                                      /* OP_POP*/
        0,                                      /* OP_PARM_DEF*/
        1,                                      /* OP_SELECT*/
        2,                                      /* OP_BIT_TEST_TRUE*/
        2,                                      /* OP_BIT_TEST_FALSE*/
        2,                                      /* OP_CMP_EQUAL*/
        2,                                      /* OP_CMP_NOT_EQUAL*/
        2,                                      /* OP_CMP_GREATER*/
        2,                                      /* OP_CMP_LESS_EQUAL*/
        2,                                      /* OP_CMP_LESS*/
        2,                                      /* OP_CMP_GREATER_EQUAL*/
        2,                                      /* OP_CALL*/
        2,                                      /* OP_SET_EQUAL*/
        2,                                      /* OP_SET_NOT_EQUAL*/
        2,                                      /* OP_SET_GREATER*/
        2,                                      /* OP_SET_LESS_EQUAL*/
        2,                                      /* OP_SET_LESS*/
        2,                                      /* OP_SET_GREATER_EQUAL*/
        0,                                      /* OP_DEBUG_INFO */
        0,                                      /* OP_CHEAP_NOP */
        1,                                      /* OP_LOAD_UNALIGNED */
        1,                                      /* OP_STORE_UNALIGNED */
        2,                                      /* OP_EXTRACT_LOW */
        2,                                      /* OP_EXTRACT_HIGH */
        2,                                      /* OP_INSERT_LOW */
        2,                                      /* OP_INSERT_HIGH */
        2,                                      /* OP_MASK_LOW */
        2,                                      /* OP_MASK_HIGH */
        2,                                      /* OP_ZAP */
        2,                                      /* OP_ZAP_NOT */
        0,                                      /* OP_STK_ALLOC */
        1,                                      /* OP_VA_START */
        0,0,0,0,0,0,0,0,0,                      /* OP_SLACK_31 .. 39 */
        0                                       /* OP_BLOCK*/
};


extern  int     NumOperands( instruction *ins ) {
/************************************************
    see NumTab
*/

    return( NumTab[ ins->head.opcode ] );
}


extern void     ReplaceOperand( instruction *ins, name *old, name *new ) {
/*************************************************************************
    Replace all occurences of operand/result "old" with "new" in "ins".
*/

    int                 i;

    i = ins->num_operands;
    while( --i >= 0 ) {
        if( ins->operands[ i ] == old ) {
            ins->operands[ i ] = new;
        }
    }
    if( ins->result == old ) {
        ins->result = new;
    }
}


extern  name    *IndexToTemp( instruction * ins, name * index ) {
/****************************************************************
        change
                 OP      foo[x], ...
        into
                 MOV     x => temp
                 OP      foo[temp], ...

*/

    name                *temp;
    instruction         *new_ins;
    name                *new_idx;
    name                *name;
    type_class_def      class;

    name = index->i.index;
    if( name->n.class == N_CONSTANT ) {
        class = WD;
    } else {
        class = name->n.name_class;
    }
    temp = AllocTemp( class );
    new_ins = MakeMove( name, temp, class );
    new_idx = ScaleIndex( temp, index->i.base,
                           index->i.constant, index->n.name_class,
                           index->n.size, index->i.scale,
                           index->i.index_flags );
    ReplaceOperand( ins, index, new_idx );
    PrefixIns( ins, new_ins );
    return( temp );
}


extern  name    *FindIndex( instruction *ins ) {
/***********************************************
    Find an N_INDEXED operand of "ins" that is not OK (Needs index
    to be split out into a temporary)
*/

    name        *index;
    int         i;

    i = 0;
    while( i < ins->num_operands ) {
        index = ins->operands[ i ];
        if( index->n.class == N_INDEXED && !IndexOkay( ins, index ) ) {
            return( index );
        }
        ++ i;
    }
    index = ins->result;
    if( index == NULL ) return( NULL );
    if( index->n.class != N_INDEXED ) return( NULL );
    if( IndexOkay( ins, index ) ) return( NULL );
    return( index );
}


extern  void    NoMemIndex( instruction *ins ) {
/***********************************************
    Make sure that we aren't using an N_MEMORY operand as the index
    of an N_INDEXED operand in "ins". If we are, call IndexToTemp.
*/

    int         i;
    name        *bad_index;

    for( ;; ) {
        i = ins->num_operands;
        bad_index = NULL;
        while( --i >= 0 ) {
            if( ins->operands[ i ]->n.class == N_INDEXED
             && ins->operands[ i ]->i.index->n.class == N_MEMORY ) {
                bad_index = ins->operands[ i ];
            }
        }
        if( ins->result != NULL ) {
            if( ins->result->n.class == N_INDEXED
             && ins->result->i.index->n.class == N_MEMORY ) {
                bad_index = ins->result;
            }
        }
        if( bad_index == NULL ) break;
        (void)IndexToTemp( ins, bad_index );
    }
}


static  name    *OpTemp( instruction *ins, uint i, uint j ) {
/************************************************************
    Used by OneMemRef to split an operand out into a temporary
*/

    name                *temp;
    type_class_def      class;
    instruction         *new_ins;

    class = FPInsClass( ins );
    if( class == XX ) {
        if( ins->operands[ i ]->n.name_class == XX ) {
            class = ins->type_class;
        } else {
            class = ins->operands[ i ]->n.name_class;
        }
    }
#if _TARGET & _TARG_AXP
    if( i == 0 && ins->head.opcode == OP_CONVERT ) {
        class = ins->base_type_class;
    }
#endif
    temp = AllocTemp( class );
    FPSetStack( temp );
    new_ins = MakeMove( ins->operands[ i ], temp, class );
    ins->operands[ i ] = temp;
    ins->operands[ j ] = temp;
    PrefixIns( ins, new_ins );
    return( temp );
}


static  instruction     *Split3( instruction *ins ) {
/****************************************************
    Used by OneMemRef to split an instruction containing 3 memory references.
*/

    name        *temp;
    instruction *new_ins;

    if( ins->operands[ 0 ]->n.class == N_INDEXED
     || ins->operands[ 0 ]->n.class == N_MEMORY ) {
        temp = OpTemp( ins, 0, 0 );
        if( temp->n.name_class != ins->result->n.name_class
          || FPIsStack( temp ) ) {
            temp = AllocTemp( ins->result->n.name_class );
            FPSetStack( temp );
        }
        new_ins = MakeMove( temp, ins->result, temp->n.name_class );
        ins->result = temp;
        SuffixIns( ins, new_ins );
        ins = new_ins;
    } else {
        OpTemp( ins, 1, 1 );
    }
    return( ins );
}


static  instruction     *ResTemp( instruction *ins ) {
/*****************************************************
    Used by OneMemRef to move the result of an instruction to an N_TEMP
    before moving it to the real result.
*/

    name        *temp;
    instruction *new_ins;

    temp = AllocTemp( ins->result->n.name_class );
    FPSetStack( temp );
    new_ins = MakeMove( temp, ins->result, temp->n.name_class );
    ins->result = temp;
    SuffixIns( ins, new_ins );
    return( new_ins );
}


extern  instruction     *OneMemRef( instruction *ins ) {
/*******************************************************
    see FixIndex
*/

    name        *op1;
    name        *op2;
    name        *res;

    FixFPConsts( ins );
    if( ins->num_operands == 2 ) {
        if( ins->result != NULL ) { /* the tough case*/
            op1 = ins->operands[ 0 ];
            if( op1->n.class != N_INDEXED && op1->n.class != N_MEMORY ) {
                op1 = NULL;
            }
            op2 = ins->operands[ 1 ];
            if( op2->n.class != N_INDEXED && op2->n.class != N_MEMORY ) {
                op2 = NULL;
            }
            res = ins->result;
            if( res->n.class != N_INDEXED && res->n.class != N_MEMORY ) {
                res = NULL;
            }
            if( op1 != NULL && op2 != NULL && res != NULL ) {
                /* three memory references*/
                if( RTLeaveOp2( ins ) ) {
                    OpTemp( ins, 0, 0 );
                    ins = ResTemp( ins );
                } else if( op1 == op2 ) {
                    OpTemp( ins, 0, 1 );
                } else if( op1 == res ) {
                    OpTemp( ins, 1, 1 );
                } else if( op2 == res ) {
                    OpTemp( ins, 0, 0 );
                } else {
                    ins = Split3( ins );
                }
            } else if( op1 == NULL && op2 == NULL ) {
                /* one memory reference*/
            } else if( op1 == NULL && res == NULL ) {
                /* one memory reference*/
            } else if( op2 == NULL && res == NULL ) {
                /* one memory reference*/
            } else if( op1 != res && op2 != res && op1 != op2 ) {
               /* two memory references*/
                if( RTLeaveOp2( ins ) ) {
                    OpTemp( ins, 0, 0 );
                    ins = ResTemp( ins );
                } else {
                    ins = Split3( ins );
                }
            }
        } else {
            op1 = ins->operands[ 0 ];
            op2 = ins->operands[ 1 ];
            if( op1->n.class != N_INDEXED && op1->n.class != N_MEMORY ) {
                return( ins );
            }
            if( op2->n.class != N_INDEXED && op2->n.class != N_MEMORY ) {
                return( ins );
            }
            if( op1 == op2 ) return( ins );
            OpTemp( ins, 0, 0 );
        }
    } else if( ins->num_operands == 1 && ins->result != NULL ) {
        op1 = ins->operands[ 0 ];
        res = ins->result;
        if( op1->n.class != N_INDEXED && op1->n.class != N_MEMORY ) {
            return( ins );
        }
        if( res->n.class != N_INDEXED && res->n.class != N_MEMORY ) {
            return( ins );
        }
        if( op1 == res ) return( ins );
        if( ins->type_class == XX ) return( ins );
        if( _IsConvert( ins ) && ins->type_class > ins->base_type_class ) {
            ResTemp( ins ); /* 90-Nov-29 */
        } else {
            OpTemp( ins, 0, 0 );
        }
    }
    return( ins );
}


extern  void    FixIndex() {
/***************************
    Make sure that there is only one memory reference per instruction.
    This is so that a segment override can tag along on the instruction
    and be unambiguous as to what it is supposed to affect.  It is also
    so that the register allocator only has to deal with an instruction
    needing at most one index register.
*/

    block       *blk;
    instruction *ins;

    blk = HeadBlock;
    while( blk != NULL ) {
        ins = blk->ins.hd.next;
        while( ins->head.opcode != OP_BLOCK ) {
            if( ins->type_class != XX
             && ins->head.opcode != OP_LA
             && ins->head.opcode != OP_CAREFUL_LA ) {
                ins = OneMemRef( ins );
            }
            ins = ins->head.next;
        }
        blk = blk->next_block;
    }
}
