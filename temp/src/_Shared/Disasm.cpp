/*
     8888888b.                  888     888 d8b                        
     888   Y88b                 888     888 Y8P                        
     888    888                 888     888                            
     888   d88P 888d888  .d88b. Y88b   d88P 888  .d88b.  888  888  888 
     8888888P"  888P"   d88""88b Y88b d88P  888 d8P  Y8b 888  888  888 
     888        888     888  888  Y88o88P   888 88888888 888  888  888 
     888        888     Y88..88P   Y888P    888 Y8b.     Y88b 888 d88P 
     888        888      "Y88P"     Y8P     888  "Y8888   "Y8888888P"  


                        Dissasembler Engine Core
                        ~~~~~~~~~~~~~~~~~~~~~~~~

 Written by Bengaly (R) 2003-2005.
 As a part of the Proview (a.k.a PVDasm).
 Permission is granted to make and distribute verbatim copies of this
 Program provided the copyright notice and this permission notice are
 Preserved on all copies.

 File: Disasm.cpp (main)

 Disassembler Core Version: 1.04b
*/  

#include "Disasm.h"



void Decode(DISASSEMBLY *Disasm,char *Opcode,DWORD *Index)
{
	/*
	  This function is the Main decoding rutine.
	  The function gets 3 params:
	  1. DISASSEMBLY struct pointer
	  2. Opcode pointer, point to the linear address to decode
	  3. Index pointer, this is the distance from the beginning<>end of the linear

	  The function First searches for Prefixes + Repeated prefixes,
	  This is the first step to do in any disasm engine.
	  Prefixes determine behavior of instruction and the way they
	  Are decoded.
	  Once prefies has been found, we changes params (such as default r/m size, Lock..)

	  The function than searched for the byte to be decoded, the actual
	  Menemonic referenced in CPU form (Opcode),
	  I have not used big table - time prob -, although it is highly recomended! (hopefully in future)
    */

	// intializations
    DWORD dwMem=0,dwOp=0;
    int  i=*Index,RegRepeat=0,LockRepeat=0,SegRepeat=0,RepRepeat=0,AddrRepeat=0; // Repeated Prefixes    
	int  RM=REG32,SEG=SEG_DS,ADDRM=REG32;       // default modes	
	int PrefixesSize=0,PrefixesRSize=0;         // PrefixesSize = all Prefixes(no rep), PrefixesRsize (with Rep Prefix)
    WORD wMem=0,wOp=0;    
    bool RegPrefix=0,LockPrefix=0,SegPrefix=0,RepPrefix=0,AddrPrefix=0;  // default size of Prefixes
    BYTE Bit_D=0, Bit_W=0; // bit d/w for R/M
	char menemonic[256]="";
    char RSize[10]="Dword";                     // default size of menemonic
    BYTE Op=(BYTE)Opcode[i];                    // current opcode
    


    //=======================================================
    //               Decoding Prefixes                     //
    //=======================================================

    // we first assume there is prefix !
	// if we skip this, our decoding might be currupted

	while( // check only RegPreifix/LockProfix/SegPrefixes/RepPrefix/AddrPerfix
		   (Op==0x66) || (Op==0x0F0)|| (Op==0x2E) || (Op==0x36) ||
		   (Op==0x3E) || (Op==0x26) || (Op==0x64) || (Op==0x65) || 
		   (Op==0xF2) || (Op==0xF3) || (Op==0x67)
		 ) 
	{
		switch(Op)
		{
			case 0x66: // reg prefix, change default size, dword->word
			{                   
				RM=REG16; // 66 prefix, change default size
				RegPrefix=1; 
				BYTE temp;

				i++;
				++(*Index);
				Op=(BYTE)Opcode[i];
				temp=(BYTE)Opcode[i+1];
				RegRepeat++;
				if(RegRepeat>1) 
				{
					Disasm->OpcodeSize=1;
					Disasm->PrefixSize=0;
					(*Index)-=RegRepeat;
					return;
				}
			}
			break;

			case 0x67: // Addr prefix, change default Reg size, (EDI->DI) and more!
			{
				ADDRM=REG16; // 67 prefix, change default size, in this case: Memory Reg Size
				AddrPrefix=1; 
				BYTE temp;

				i++;
				++(*Index);
				Op=(BYTE)Opcode[i];
				temp=(BYTE)Opcode[i+1];
				AddrRepeat++;
				if(AddrRepeat>1) 
				{
					Disasm->OpcodeSize=1;
					Disasm->PrefixSize=0;
					(*Index)-=AddrRepeat;
					return;
				}
			}
			break;

			case 0x0F0: // LockPrefix, Add bus lock menemonic opcode in front of every menemonic
			{
				//BYTE temp;
				LockPrefix=1;
				//RegPrefix=0; 

				i++;
				++(*Index);
				Op=(BYTE)Opcode[i];
				//temp=(BYTE)Opcode[i+1];
				LockRepeat++;
				if(LockRepeat>1) 
				{
					Disasm->OpcodeSize=1;
					Disasm->PrefixSize=0;
					(*Index)-=LockRepeat;
					return;
				}
			}
			break;

			case 0xF2: case 0xF3: // RepPrefix (only string instruction!!)
			{
                BYTE NextOp=(BYTE)Opcode[i+1];              // Next followed opcode
                BYTE NextOp2=(BYTE)Opcode[i+2];
				RepPrefix=1;

				i++;
				++(*Index);
				Op=(BYTE)Opcode[i];
				RepRepeat++;
				// REPE/REPNE Prefixes affect only string operations:
				// MOVS/LODS/SCAS/CMPS/STOS/CMPSS.CMPPS..etc (NewSet Instructions)
				if(!( 
                      (Op>=0xA4 && Op<=0xA7) || 
                      (Op>=0xAA && Op<=0xAF) || 
                      (NextOp==0x0F && NextOp2==0x2A) ||
                      (NextOp==0x0F && NextOp2==0x10) ||
                      (NextOp==0x0F && NextOp2==0x11) ||
                      (NextOp==0x0F && NextOp2==0x2C) ||
                      (NextOp==0x0F && NextOp2==0x2D) ||
                      (NextOp==0x0F && NextOp2==0x51) ||
                      (NextOp==0x0F && NextOp2==0x52) ||
                      (NextOp==0x0F && NextOp2==0x53) ||
                      (NextOp==0x0F && NextOp2==0x58) ||
                      (NextOp==0x0F && NextOp2==0x59) ||
                      (NextOp==0x0F && NextOp2==0x5C) ||
                      (NextOp==0x0F && NextOp2==0x5D) ||
                      (NextOp==0x0F && NextOp2==0x5E) ||
                      (NextOp==0x0F && NextOp2==0x5F) ||
                      (NextOp==0x0F && NextOp2==0xC2) 
                    )
                  )
				{
					Disasm->OpcodeSize=1;
					Disasm->PrefixSize=0;
					(*Index)-=RepRepeat;
					return;
				}
					
			}
			break;

			case 0x2E: case 0x36: // Segment Prefixes
			case 0x3E: case 0x26: // Segment Prefixes
			case 0x64: case 0x65: // Segment Prefixes
			{
				BYTE temp;
				switch(Op)
				{
					// Change Default Segment
					case 0x2E: SEG = SEG_CS; break; // Segment CS
					case 0x36: SEG = SEG_SS; break; // Segment SS
					case 0x3E: SEG = SEG_DS; break; // Segment DS
					case 0x26: SEG = SEG_ES; break; // Segment ES
					case 0x64: SEG = SEG_FS; break; // Segment FS
					case 0x65: SEG = SEG_GS; break; // Segment GS
				}

				SegPrefix=1;

				i++;
				++(*Index);
				Op=(BYTE)Opcode[i];
				temp=(BYTE)Opcode[i-2];
				SegRepeat++;

				// Check if SegPrefix is repeating
				if(SegRepeat>1) 
				{                    
					BYTE opc=(BYTE)Opcode[i-1];

					if(  temp==0x2E || temp==0x36 ||
					     temp==0x3E || temp==0x26 ||
                         temp==0x64 || temp==0x65 ||
                         temp==0x66 || temp==0xF0 ||
                         temp==0x67                        
					  )
					{
						// Check if last byte was an seg prefix and show it
						if(temp==0x66 || temp==0xF0 || temp==0x67)
                        {
                           opc=(BYTE)Opcode[i-3];
                           SegRepeat++;
                        }
                        else
                           opc=(BYTE)Opcode[i-2];

						switch(opc)
						{
							// Change Segment, accurding to last segPrefix (if repeated)
							case 0x2E: SEG = SEG_CS; break; // Segment CS
							case 0x36: SEG = SEG_SS; break; // Segment SS
							case 0x3E: SEG = SEG_DS; break; // Segment DS
							case 0x26: SEG = SEG_ES; break; // Segment ES
							case 0x64: SEG = SEG_FS; break; // Segment FS
							case 0x65: SEG = SEG_GS; break; // Segment GS
						}
					
                        Disasm->OpcodeSize=0;
                        Disasm->PrefixSize=1;
                        (*Index)-=SegRepeat;
                    }
					return;
				}
			}
			break;
			
		    default:
			{
				// reset prefixes/repeats to default
				LockRepeat=0;
				RegRepeat=0;
				SegRepeat=0;
				RegPrefix=0;
				LockPrefix=0;
				SegPrefix=0;
			}
			break;
		}
	}

    
	//=============================================
	//        Main Decoding starts here!         //
	//=============================================

	// Calculate Prefixes Sizes
	PrefixesSize  = RegPrefix+LockPrefix+SegPrefix+AddrPrefix; // No RepPrefix
	PrefixesRSize = PrefixesSize+RepPrefix; // Special Case (Rep Prefix is being used -> String menemonics only)

    switch(Op) // Find & Decode Big Set Opcodes
    {
		case 0x00: case 0x01: case 0x02: case 0x03: // ADD  XX/XXX, XX/XXX
		case 0x08: case 0x09: case 0x0A: case 0x0B: // OR   XX/XXX, XX/XXX
		case 0x10: case 0x11: case 0x12: case 0x13: // ADC  XX/XXX, XX/XXX 
		case 0x18: case 0x19: case 0x1A: case 0x1B: // SBB  XX/XXX, XX/XXX 
		case 0x20: case 0x21: case 0x22: case 0x23: // AND  XX/XXX, XX/XXX 
		case 0x28: case 0x29: case 0x2A: case 0x2B: // SUB  XX/XXX, XX/XXX 
		case 0x30: case 0x31: case 0x32: case 0x33: // XOR  XX/XXX, XX/XXX 
		case 0x38: case 0x39: case 0x3A: case 0x3B: // CMP  XX/XXX, XX/XXX 
        case 0x88: case 0x89: case 0x8A: case 0x8B: // MOV  XX/XXX, XX/XXX 
        case 0x8C: case 0x8E:                       // MOV  XX/XXX, XX/XXX
		case 0x62: case 0x63:                       // BOUND / ARPL XX/XXX, XX/XXX
        case 0x69:                                  // IMUL RM,IIM32 (DWORD)
        case 0x6B:                                  // IMUL <reg>,<RM>
		case 0x80: case 0x81: case 0x82: case 0x83: // MIXED Instructions
		case 0x84: case 0x85:                       // TEST
		case 0x86: case 0x87:                       // XCHG
        case 0x8D:                                  // LEA 
        case 0x8F:                                  // POP
        case 0xC0: case 0xC1:                       // MIXED Instructions
        case 0xC4: case 0xC5:                       // LES / LDS REG,MEM
        case 0xC6: case 0xC7:                       // MOV [MEM],IIM8/16/32
        case 0xD0: case 0xD1: case 0xD2: case 0xD3: // MIXED Bitwise Instructions
        case 0xD8: case 0xD9: case 0xDA: case 0xDB: // FPU Instructions
        case 0xDC: case 0xDD: case 0xDE: case 0xDF: // FPU Instructions
        case 0xF6: case 0xF7: case 0xFE: case 0xFF: // MIX Instructions
        {
			if(((BYTE)Opcode[i+1] & 0xC0)==0xC0)   // Check Opcode Range
			{				
				Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
				Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
				
				// Check Special Cases for alone Opcodes
				switch(Op)
				{
                    case 0x63:{Bit_D=0;Bit_W=1;}               break;
                    case 0x62:{Bit_D=1;Bit_W=1;}               break;
                    case 0x86:{Bit_D=0;Bit_W=0;}               break;
                    case 0x87:{Bit_D=0;Bit_W=1;}               break;
                    case 0x80: case 0x82: { Bit_D=0;Bit_W=0; } break;
                    case 0x81: case 0x83: { Bit_D=0;Bit_W=1; } break;
                    case 0x8C:{ Bit_D=0;Bit_W=0;}              break;
                    case 0x8E:{ Bit_D=1;Bit_W=0;}              break;
                    case 0xC4: case 0xC5: { Bit_D=1;Bit_W=1; } break;
				}

				Mod_11_RM(Bit_D,Bit_W,&Opcode,&Disasm,RegPrefix,Op,&Index); // Decode with bits
				Disasm->PrefixSize=PrefixesSize; // PrefixSize (if prefix present)				
				break;
			}
			
			// operand doesn't have byte(s) extension in addressing mode
			if((BYTE)Opcode[i+1]>=0x00 && (BYTE)Opcode[i+1]<=0xBF)
			{
				Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
				Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
				Mod_RM_SIB(&Disasm,&Opcode,i,AddrPrefix,SEG,&Index,Bit_D,Bit_W,Op,RegPrefix,SegPrefix,AddrPrefix);
				Disasm->PrefixSize=PrefixesSize;
				break;
			}	  				
		}
		break;

		case 0x04:case 0x0C:case 0x14: // INSTRUCTION AL,XX
		case 0x1C:case 0x24:case 0x2C: // INSTRUCTION AL,XX
        case 0x34:case 0x3C:case 0xA8: // INSTRUCTION AL,XX
		case 0xE4:
        {
			Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
            ++(*Index);
        }
        break;

        case 0x05:case 0x0D:case 0x15: // INSTRUCTION EAX/AX,XXXXXXXX
        case 0x1D:case 0x25:case 0x2D: // INSTRUCTION EAX/AX,XXXXXXXX
		case 0x35:case 0x3D:case 0xA9: // INSTRUCTION EAX/AX,XXXXXXXX
        {
			if(RegPrefix==0) // no prefix
			{   
				// read 4 bytes into EAX
				SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
			else if(RegPrefix==1) // RegPrefix is being used
			{   
				// read 2 bytes into AX (REG16)
				SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=2;
			}
        }
        break;

        case 0x06: // PUSH ES
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x07: // POP ES
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x0E: // PUSH CS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

        // INTEL - NEW SET (MMX/3DNow!/SSE/SSE2)
/*        case 0x0F:
        {
          int RetVal;
          BYTE Code=(BYTE)Opcode[i+1];

          RetVal=GetNewInstruction(Code);

          switch(RetVal) // check if we need to decode instruction
          {
            case 0:
            {
                // Decode SIB + ModRM
                if((BYTE)Opcode[i+2]>=0x00 && (BYTE)Opcode[i+2]<=0xBF)
                {
                    (*Index)++;
                    i=*Index;
                    Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
				    Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
                    Mod_RM_SIB_EX(&Disasm,&Opcode,i,AddrPrefix,SEG,&Index,Code,RegPrefix,SegPrefix,AddrPrefix,Bit_D,Bit_W,RepPrefix);
                    Disasm->PrefixSize=PrefixesSize;
                    Disasm->OpcodeSize++; // 0F extra Byte
                    break;
			    }
                else
                {
                    if(((BYTE)Opcode[i+2] & 0xC0)==0xC0)
                    {                    
                        Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
                        Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
                        (*Index)++;
                        i=*Index;
                        Mod_11_RM_EX(Bit_D,Bit_W,&Opcode,&Disasm,RegPrefix,Code,&Index,RepPrefix); // Decode with bits
                        Disasm->PrefixSize=PrefixesSize;
                        Disasm->OpcodeSize++; // 0F extra Byte
                    }
                    break;
                }
            }
            break; // big set instructions
            
            case 1: // 1 byte instructions set
            {
                Disasm->OpcodeSize=2;
                Disasm->PrefixSize=PrefixesSize;
                (*Index)++;
            }
            break;
            
            case 2: // NEAR JUMP (JXX)
            {
                SwapDword((BYTE*)(Opcode+i+2),&dwOp,&dwMem);
                dwMem+=Disasm->Address+PrefixesSize+6; // calculate dest addr

				Disasm->OpcodeSize=6;
                Disasm->PrefixSize=PrefixesSize;
                (*Index)+=5;
                
            }
            break; // jump instructions set

            case 3:
            {
                if(((BYTE)Opcode[i+2]&0xC0)==0xC0)
                {
                    Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
                    Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
                    (*Index)++;
                    i=*Index;
                    Mod_11_RM_EX(Bit_D,Bit_W,&Opcode,&Disasm,RegPrefix,Code,&Index,RepPrefix); // Decode with bits
                    Disasm->PrefixSize=PrefixesSize;
                    Disasm->OpcodeSize++; // 0F extra Byte
                }
                else
                {
                    Disasm->OpcodeSize=2;
                    Disasm->PrefixSize=PrefixesSize;
                    (*Index)++;
                }
            }
            break;

			case 4:
			{
				if( ((BYTE)Opcode[i+2]>=0x08) && ((BYTE)Opcode[i+2]<=0x0F) )
				{
                    (*Index)++;
                    i=*Index;
                    Bit_D=(Op&0x02)>>1;      // Get bit d (direction)
				    Bit_W=(Op&0x01);         // Get bit w (full/partial reg size)
                    Mod_RM_SIB_EX(&Disasm,&Opcode,i,AddrPrefix,SEG,&Index,Code,RegPrefix,SegPrefix,AddrPrefix,Bit_D,Bit_W,RepPrefix);
                    Disasm->PrefixSize=PrefixesSize;
                    Disasm->OpcodeSize++; // 0F extra Byte
				}
				else
				{
					Disasm->OpcodeSize=2;
					Disasm->PrefixSize=PrefixesSize;
					(*Index)++;
				}
			}
			break;
          }
          
          
        }
        break;
*/

		case 0x16: // PUSH SS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x17: // POP SS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x1E: // PUSH DS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x1F: // POP DS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x27: // DAA
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x2F: // DAS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x37: // AAA
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x3F: // AAS
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x40:case 0x41: // INC XXX/XX
		case 0x42:case 0x43: // INC XXX/XX
		case 0x44:case 0x45: // INC XXX/XX
		case 0x46:case 0x47: // INC XXX/XX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x48:case 0x49: // DEC XXX/XX
		case 0x4A:case 0x4B: // DEC XXX/XX
		case 0x4C:case 0x4D: // DEC XXX/XX
		case 0x4E:case 0x4F: // DEC XXX/XX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x50:case 0x51: // PUSH XXX/XX
		case 0x52:case 0x53: // PUSH XXX/XX
		case 0x54:case 0x55: // PUSH XXX/XX
		case 0x56:case 0x57: // PUSH XXX/XX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x58:case 0x59: // POP XXX/XX
		case 0x5A:case 0x5B: // POP XXX/XX
		case 0x5C:case 0x5D: // POP XXX/XX
		case 0x5E:case 0x5F: // POP XXX/XX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x60: // PUSHAD/W (Prefix)
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x61: // POPAD/W (Prefix) 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x68: // PUSH XXXXXXXX
        {
			if(RegPrefix==0)
			{   // PUSH 4 bytes
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
			else 
			{
				// PUSH 2 bytes
				SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=2;
			}
        }
        break;

		case 0x6A: // PUSH XX
        {
            Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
            ++(*Index);
        }
        break;

		case 0x6C: case 0x6D: // INSB/INSW/INSD
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x6E: case 0x6F: // OUTSB/OUTSW/OUTSD
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x70: case 0x71: case 0x72: case 0x73: //JUMP XXXXXXXX
		case 0x74: case 0x75: case 0x76: case 0x77: //JUMP XXXXXXXX 
		case 0x78: case 0x79: case 0x7A: case 0x7B: //JUMP XXXXXXXX
		case 0x7C: case 0x7D: case 0x7E: case 0x7F: //JUMP XXXXXXXX
		case 0xE0: case 0xE1: case 0xE2: case 0xEB: //JUMP XXXXXXXX
        case 0xE3:
        {
			DWORD JumpAddress=0;
			BYTE JumpSize;
			JumpSize=(BYTE)Opcode[i+1];

            // Short Jump $+2
			if((BYTE)Opcode[i+1]>0x7F)
				JumpAddress=Disasm->Address + ((2 + PrefixesSize + JumpSize)-0x100);
			else
				JumpAddress=Disasm->Address + 2 + JumpSize  +PrefixesSize;
			
			Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
			++(*Index);		
        }
        break;

		case 0x90: // NOP (XCHG EAX, EAX) 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x91:case 0x92: // XCHG XXX, XXX
		case 0x93:case 0x94: // XCHG XXX, XXX
		case 0x95:case 0x96: // XCHG XXX, XXX
		case 0x97:           // XCHG XXX, XXX
		{
			Mod_11_RM(1,1,&Opcode,&Disasm,RegPrefix,Op,&Index);//+ 0x30			
			Disasm->PrefixSize=PrefixesSize;
		}
		break;

		case 0x98: // CWDE/CDW (Prefix) 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x99: // CWDE/CDW (Prefix) 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x9A: case 0xEA: // CALL/JMP XXXX:XXXXXXXX (FAR CALL)
		{
			if(AddrPrefix==0)
			{
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);
                SwapWord((BYTE*)(Opcode+i+5),&wOp,&wMem);

				Disasm->OpcodeSize=7;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=6;
			}
			else
			{
                WORD w_op,w_mem;
                SwapWord((BYTE*)(Opcode+i+3),&wOp,&wMem);
                SwapWord((BYTE*)(Opcode+i+1),&w_op,&w_mem);  

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
		}
		break;

		case 0x9B: // WAIT
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x9C: // PUSHFD/PUSHFW 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x9D: // POPFD/POPFW 
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x9E: // SAHF
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0x9F: // LAHF
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xA0:case 0xA2: // MOV AL, BYTE PTR XX:[XXXXXXXX], AL
		{
			if(!AddrPrefix)
			{
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
			else 
			{
                SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=2;
			}
		}
		break;

		case 0xA1:case 0xA3: // MOV EAX/AX, BYTE PTR XX:[XXXXXXXX], EAX/AX
		{
			if(!AddrPrefix)// no addr size change
			{
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
			else if(AddrPrefix==1)
			{
                SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=2;
			}						
		}
		break;

        case 0xA4:case 0xA5: // MOVSB/MOVSW/MOVSD
        {
			Disasm->PrefixSize=PrefixesRSize;
        }
        break;

		case 0xA6:case 0xA7: // CMPSB/CMPSW/CMPSD
        {
			Disasm->PrefixSize=PrefixesRSize;
        }
        break;

		case 0xAA:case 0xAB: // STOSB/STOSW/STOSD
        {
			Disasm->PrefixSize=PrefixesRSize;
        }
        break;

		case 0xAC:case 0xAD: // LODSB/LODSW/LODSD
        {
			Disasm->PrefixSize=PrefixesRSize;
        }
        break;

		case 0xAE:case 0xAF: // SCASB/SCASW/SCASD
        {
			Disasm->PrefixSize=PrefixesRSize;
        }
        break;

		case 0xB0:case 0xB1: // MOV XX, XX
	    case 0xB2:case 0xB3: // MOV XX, XX
		case 0xB4:case 0xB5: // MOV XX, XX
		case 0xB6:case 0xB7: // MOV XX, XX
        {
            Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
            ++(*Index);
        }
        break;

		case 0xB8:case 0xB9: // MOV XX/XXX, XXXXXXXX
		case 0xBA:case 0xBB: // MOV XX/XXX, XXXXXXXX
		case 0xBC:case 0xBD: // MOV XX/XXX, XXXXXXXX
		case 0xBE:case 0xBF: // MOV XX/XXX, XXXXXXXX
		{
			if(!RegPrefix) // check if default prefix has changed
			{
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=4;
			}
			else
			{
                SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize=PrefixesSize;
				(*Index)+=2;
			}
		}
		break;

		case 0xC2:case 0xCA: // RET/F XXXX
		{            
            SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

			Disasm->OpcodeSize=3;
			Disasm->PrefixSize=PrefixesSize;

			(*Index)+=2;
		}
		break;

		case 0xC3: // RET
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xC8: // ENTER XXXX, XX
		{
            SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);

			Disasm->OpcodeSize=4;
			Disasm->PrefixSize=PrefixesSize;
			(*Index)+=3;
        }
		break;

		case 0xC9: // LEAVE
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xCB: // RETF
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xCC: // INT 3
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xCD: // INT XX
        {
            Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
            ++(*Index);
        }
        break;

		case 0xCE: // INTO
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xCF: // IRETD/W
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xD4:case 0xD5: // AAM/AAD
        {
            Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
            ++(*Index);
        }
        break;

		case 0xD6: // SALC
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xD7: // XLAT
		{
			Disasm->PrefixSize=PrefixesSize;
		}
		break;

		// 0xE4 is at 'in al, xxx' cases
		case 0xE5: // IN EAX/AX, XX
        {
			Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;
			++(*Index);
        }
        break;

		case 0xE6: // OUT XX, AL
        {
            Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;

			++(*Index);
        }
        break;

		case 0xE7: // OUT XX, AX/EAX
        {
			Disasm->OpcodeSize=2;
			Disasm->PrefixSize=PrefixesSize;

			++(*Index);
        }
        break;

		case 0xE8:case 0xE9: // CALL/JMP XXXX/XXXXXXXX
		{
			DWORD CallAddress=0;
			DWORD CallSize=0;

			if(!RegPrefix)
			{
                SwapDword((BYTE*)(Opcode+i+1),&dwOp,&dwMem);				
				dwMem+= Disasm->Address + CallSize + 5 + (PrefixesSize-RegPrefix);

				Disasm->OpcodeSize=5;
				Disasm->PrefixSize = PrefixesSize;                
				(*Index)+=4;
			}
			else 
			{
                SwapWord((BYTE*)(Opcode+i+1),&wOp,&wMem);                
				if(wMem>=0x0000F000)
					CallAddress = (wMem + 4 + (PrefixesSize-RegPrefix))-0x0000F000;
				else
					CallAddress = (Disasm->Address-0x00400000) + wMem + 4 + (PrefixesSize-RegPrefix);

				Disasm->OpcodeSize=3;
				Disasm->PrefixSize = PrefixesSize;
				(*Index)+=2;
			}
		}
		break;

		case 0xEC: // IN AL, DX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xED: // IN AX/EAX, DX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xEE: // OUT DX, AL
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xEF: // OUT DX, AX/EAX
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xF1: // ICEBP (INT1)
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xF4: // HLT (HALT)
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xF5: // CMC
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xF8: // CLC
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xF9: // STC
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xFA: // CLI
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xFB: // STI
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xFC: // CLD
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;

		case 0xFD: // STD
        {
			Disasm->PrefixSize=PrefixesSize;
        }
        break;
    }
}

void FlushDecoded(DISASSEMBLY *Disasm)
{
	// Clear all information of an decoded 
	// Instruction

    Disasm->OpcodeSize=1;        // Smallest opcode size
	Disasm->PrefixSize=0;        // No Prefixes
}
