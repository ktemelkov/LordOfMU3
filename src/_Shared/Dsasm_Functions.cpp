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

 File: Dsasm_Functions.cpp (main)

 Disassembler Core Version: 1.04b
*/


#include "Disasm.h"


// =============================================//
//               Decoding Functions             //
// =============================================//

void Mod_11_RM(BYTE d, BYTE w,char **Opcode,DISASSEMBLY **Disasm, bool PrefixReg,BYTE Op,DWORD **index)
{
	/* 
       Function Mod_11_RM Checks whatever we have
	   Both bit d (direction) and bit w (full/partial size).
	 
       There are 4 states:
	   00 - d=0 / w=0 ; direction -> (ie: DH->DL),   partial size (AL,DH,BL..)
	   01 - d=0 / w=1 ; direction -> (ie: EDX->EAX), partial size (EAX,EBP,EDI..)
	   10 - d=1 / w=0 ; direction <- (ie: DH<-DL),   partial size (AL,DH,BL..)
	   11 - d=1 / w=1 ; direction <- (ie: EDX<-EAX), partial size (EAX,EBP,EDI..)
	
       Also deals with harder opcodes which have diffrent
       Addresing type.
    */

    DWORD dwMem=0,dwOp=0;
	int RM,IndexAdd=1,m_OpcodeSize=2,Pos; // Register(s) Pointer
    WORD wMem=0,wOp=0;
	BYTE reg1=0,reg2=0,m_Opcode=0,REG;
    BYTE FOpcode;
    
    Pos=(*(*index)); // Current Position
    
    m_Opcode = (BYTE)(*(*Opcode+Pos+1));// Decode registers from second byte
    
    // Strip Used Instructions / Used Segment
    REG=(BYTE)(*(*Opcode+Pos+1)); 
    REG>>=3;
	REG&=0x07;

    // Check Opcode range
    if((Op>=0x80 && Op<=0x83) || Op==0xC7 || Op==0x69)
    {    
        switch(Op) // Find Current Opcode
        {
            // Diffrent Opcodes ahs different Modes

            case 0x80: case 0x82: case 0x83:// 1 byte
            {
                RM=REG8;
                if(Op==0x83 && PrefixReg==0) // full size reg
                    RM=REG32;
				
				if(PrefixReg==1)
					RM=REG16;
				
                reg1=(m_Opcode&7); // Get Destination Register                
                SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);
                FOpcode=wOp&0x00FF;
                
                if (FOpcode > 0x7F) // check for signed numbers!!
                    FOpcode = 0x100 - FOpcode; // -XX

                m_OpcodeSize=3;
                (*(*index))+=2; // Prepare to read next Instruction
            }
            break;
            
            case 0x81: case 0xC7: case 0x69: // 2 (WORD)/4 (DWORD) bytes
            {
                // 0x66 is being Used
                if(PrefixReg==1) // READ WORD
                {
                    RM=REG16;
                    reg1=(m_Opcode&0x07); // Get Destination Register
                    SwapWord((BYTE*)(*Opcode+Pos+2),&wOp,&wMem);
                    SwapDword((BYTE*)(*Opcode+Pos),&dwOp,&dwMem);
                    // Read imm16

					// Read Opcodes: Opcode - imm16

					m_OpcodeSize=4; // Instruction Size
                    (*(*index))+=3;
                }
                else // READ DWORD
                {
                    RM=REG32;
                    reg1=(m_Opcode&0x07); // Get Destination Register                    
                    SwapDword((BYTE*)(*Opcode+Pos+2),&dwOp,&dwMem);
                    SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
                    // Read Dword Data number (imm32)
                    // Read Opcodes: Opcode - imm32

					m_OpcodeSize=6; // Instruction Size
                    (*(*index))+=5;                    
                }                
            }
            break;
        }
        
        if(Op==0x69)
            reg2=((m_Opcode&0x038)>>3);
        
        (*Disasm)->OpcodeSize = m_OpcodeSize;
        return; // RET
        
    }
    else{ // Check Other Set of Opcodes        
        
        // Special Types usnig Segments
        if (Op==0x8C || Op==0x8E)
        {
            RM=REG16;
            reg1=(m_Opcode&0x07);
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
                        
            // Add data to the Struct
            (*Disasm)->OpcodeSize=2; // Instruction Size

            (*(*index))++;
            return;
        }

		if(Op==0xC6)
		{
			RM=REG8;
			if(m_Opcode>=0xC0 && m_Opcode<=0xC7)
			{
				reg1=(m_Opcode&0x07); // Get Destination Register
				SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);

				// Read imm16
				// Read Opcodes: Opcode - imm16                   
				m_OpcodeSize=3; // Instruction Size
				(*(*index))+=2;
			}
			else
			{
				SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);
				m_OpcodeSize=3;
				(*(*index))+=2;
			}

			(*Disasm)->OpcodeSize=m_OpcodeSize;
			return;
		}

        // Mixed Instructions
        if(Op==0xC0 || Op==0xC1)
        {
            // Check register Size
            if(w==0) 
                RM=REG8;
            else
            {
                if(PrefixReg==1)
                    RM=REG16;
                else
                    RM=REG32;
            }       

            reg1=(m_Opcode&7); // Get Destination Register
            SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);

			// Read Opcodes: Opcode - imm8
            m_OpcodeSize=3;
            (*(*index))+=2; // Prepare to read next Instruction

            (*Disasm)->OpcodeSize=m_OpcodeSize;
            return; // exit the function
        }

        // XCHG Register
        if(Op>=0x91 && Op<=0x97) 
        {
            m_Opcode=(*(*Opcode+Pos)); // 1 byte Opcode
            m_Opcode+=0x30;            // Add 0x30 in order to get values of EAX-EDI (trick)
            IndexAdd=0;                // Dont Add to the index counter.
            m_OpcodeSize=1;            // 1 byte opcode          
        }

        // (->) / reg8
        if(d==0 && w==0)
        {    
            RM=REG8;
            reg1=(m_Opcode&0x07);
            reg2=(m_Opcode&0x38)>>3;
        }
        
        // (->) / reg32
        if(d==0 && w==1)
        {    
            RM=REG32;
            if(PrefixReg==1)
                RM=REG16; // (->) / reg16 (RegPerfix is being used)

            reg1=(m_Opcode&0x07);
            reg2=(m_Opcode&0x38)>>3;
        }
        
        // (<-) / reg8
        if(d==1 && w==0)
        {    
            RM=REG8;
            reg2=(m_Opcode&0x07);
            reg1=(m_Opcode&0x38)>>3;
        }
        
        // (<-) / reg32
        if(d==1 && w==1)
        {    
            RM=REG32;
            if(PrefixReg==1)
                RM=REG16; // (<-) / reg16

           reg2=(m_Opcode&0x07);
           reg1=(m_Opcode&0x38)>>3;
        }
        
        // Check Opcode Size (XCHG changes it)
       if (m_OpcodeSize!=1)
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

       switch(Op) 
       {
         case 0x6B: // IMUL REG,REG,IIM
         {
             SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);

             FOpcode = wOp & 0x00FF;

             if(FOpcode>0x7F) // check for signed numbers!!
                 FOpcode = 0x100-FOpcode; // -XX (Signed)

             m_OpcodeSize=3;
             (*(*index))++;
         }
         break;
                    
         case 0xF6:
         {             
             if(reg1==0 || reg1==1)
             {
                 SwapWord((BYTE*)(*Opcode+Pos+1),&wOp,&wMem);
                 (*(*index))++;
                 m_OpcodeSize++;
             }
         }
         break;

         case 0xF7:
         {             
           if(reg1==0 || reg1==1)
           {
               if(!PrefixReg) // no 0x66 prefix used (read DWORD)
               {
                 SwapDword((BYTE*)(*Opcode+Pos+2),&dwOp,&dwMem);
                  (*(*index))+=4; 
                  m_OpcodeSize+=4;
               }
               else // prefix 0x66 is being used (read WORD)
               {
                   SwapWord((BYTE*)(*Opcode+Pos+2),&wOp,&wMem);
                   (*(*index))+=2;
                   m_OpcodeSize+=2;
               }                          
           }
         }
         break;

       }

       (*Disasm)->OpcodeSize=m_OpcodeSize;
       (*(*index))+=IndexAdd;
    }

    return; // RET
}


void Mod_RM_SIB(
		  DISASSEMBLY **Disasm,
		  char **Opcode, int pos, 
		  bool AddrPrefix,
		  int SEG,
		  DWORD **index,
		  BYTE Bit_d, 
		  BYTE Bit_w, 
		  BYTE Op,
		  bool PrefixReg,
		  bool PrefixSeg,
		  bool PrefixAddr
		 )
{
    /*
        This Function will resolve BigSet menemonics: 
        ADC, ADD, AND, CMP, MOV, OR, SBB, SUB, XOR,ARPL, BOUND..
        We analyze the opcode using ;
        BitD, BitW,SIB ( SS III BBB : Sacle-Index-Base)
        MOD/RM
    */

	// Set Defaults    
    DWORD dwOp,dwMem;
	int RM=REG8,SCALE=0,SIB,ADDRM=REG32;
    WORD wOp,wMem;
    bool bound=0,UsesFPU=0;
	BYTE reg1=0,reg2=0,REG=0,Extension=0,FOpcode=0;
	

    // Get used Register
	// Get target register, example:
	// 1. add byte ptr [ecx], -> al <-
	// 2. add -> al <- ,byte ptr [ecx]
    REG=(BYTE)(*(*Opcode+pos+1)); 
	REG>>=3;
	REG&=0x07;

    //Displacement MOD (none|BYTE/WORD|DWORD)
	Extension=(BYTE)(*(*Opcode+pos+1))>>6;
	/*
	    There are 3 types of Displacement to RegMem
	    00 -> [00] 000 000 ; no byte extention ([RegMem])
	    40->  [01] 000 000 ; 1 byte extenton ([RegMem+XX])
	    80 -> [10] 000 000 ; 4 bytes extention ([RegMem+XXXXXXXX])
	*/

    //===================//
    // Bitwise OverRides //
    //===================//
	// Arpl, Bound, Test, Xchg menemonics are special cases! when alone.
	// so we need to set specific static bits for d/w
    // We specify Size of Data corresponding to each mnemonic.

	switch((BYTE)(*(*Opcode+pos)))
	{
      case 0x20:            { PrefixReg=0;                                        }     break; // Force Byte Size Regardless Operands.
      case 0x39: case 0x3B:     break; // DWORD
      case 0x63:            { Bit_d=0; Bit_w=1; }             break; // DWORD
	  case 0x62:            { RM=REG32; bound=1; Bit_d=1; Bit_w=0; }    break; // QWORD
      case 0x69:            { Bit_d=0; Bit_w=1; }     break; // DWORD
      case 0x6B:            { Bit_d=0; Bit_w=1; }     break; // DWORD
      case 0x84: case 0x86: { Bit_d=0; Bit_w=0; }                                       break; // BYTE
      case 0x85: case 0x87: { Bit_d=0; Bit_w=1; }             break; // DWORD
      case 0x80: case 0x82: case 0xC6: case 0xF6: { Bit_d=0;Bit_w=0; } break; // BYTE
      case 0x81: case 0x83: case 0xC7: case 0xF7: case 0x89:{ Bit_d=0;Bit_w=1; } break;	
      case 0x8B: break; // DWORD
      case 0x8C: case 0x8E: { }                               break; // WORD
      case 0x8D: case 0x8F: { Bit_d=1; Bit_w=1; }             break; // POP/LEA
      case 0xC0:            { Bit_d=1; Bit_w=0;}                                        break; // BYTE
      case 0xC1:            { Bit_d=1; Bit_w=1; }             break; // MIX
      case 0xC4: case 0xC5: { RM=REG32; Bit_d=1; Bit_w=0; }             break; // LES/LDS
      case 0xD0: case 0xD2: { Bit_d=0; Bit_w=0; }             break; // MIX
      case 0xD1: case 0xD3: { Bit_d=0; Bit_w=1; }             break; // MIXED
      case 0xD8:            { UsesFPU=1; Bit_d=0; Bit_w=1; }  break; // FPU
      case 0xD9:{ 
                  UsesFPU=1; Bit_d=0; Bit_w=0; 
                }
                break; // FPU

      case 0xDA: { UsesFPU=1; Bit_d=0; Bit_w=1; }                                         break; // FPU
      case 0xDB: { UsesFPU=1; Bit_d=0; Bit_w=0; } break; // FPU
      case 0xDC: { UsesFPU=1; Bit_d=0; Bit_w=0; }                                         break; // FPU
      case 0xDD: { 
                   UsesFPU=1; Bit_d=0; Bit_w=0;
                 }
                 break; // FPU

      case 0xDE: { UsesFPU=1; Bit_d=0; Bit_w=0; }     break; // WORD
      case 0xDF: {
                   UsesFPU=1; Bit_d=0; Bit_w=0;
                 }
                 break;
      case 0xFE: { Bit_d=0; Bit_w=0; }                 break; // BYTE
      case 0xFF: { 
                   Bit_d=0; Bit_w=0; 
                 }
                 break; // DWORD

    }

	// check for bit register size : 16bit/32bit
	if(Bit_w==1)
	{
	   RM=REG32; // 32bit registers set       

	   //if(/*!bound/&& Op==0x62*/)// Special Case           
	}

	
    // SCALE INDEX BASE :
	SIB=(BYTE)(*(*Opcode+pos+1))&0x07; // Get SIB extension
	/*
	   Exmaple:
	   --------
       
       format of sib is:
       ss iii bbb.
       where ss is 2 upper bits for scale
       and they represent power (exponent) of 2 for
       scale index multipyer.
       iii is 3 middle bits for index.
       bbb is 3 low bits for base.

       *SIB == 4
	   *NO SIB != 4

       0x04 -> 00 000 [100] <- SIB
	   0x0C -> 00 001 [100] <- SIB
	   0x64 -> 01 100 [100] <- SIB
	   0x60 -> 01 100 [000] <- NO SIB
	   0xB5 -> 10 110 [101] <- NO SIB
	   0x76 -> 01 110 [110] <- NO SIB

       Extract SS II BB information (3rd byte)
       =======================================
       0x81,0xAC,0x20

       0x20 =  00 100 000
       
       Scale: 00 = *1 (not shown)
       100 - ESP = not Shown, Cannot be an Index register
       000 - EAX = shown

       if MOD 10/01 is being used, get displacement data after 
       the SIB.
	*/


    // ===================================================//
    //             AddrPrefix is being used!               //
    // ===================================================//

	if(PrefixAddr==1) // Prefix 0x67 is set, Change Segments/Addressing Modes to 16 bits
	{		
        FOpcode=((BYTE)(*(*Opcode+pos+1))&0x0F); // Get addressing Mode (8 types of mode)
		reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3;

        
        // Choose Mode + Segment
		switch(FOpcode)
		{
		  case 0x00: case 0x08: /*SEG=SEG_DS;*/ break; // Mode 0:[BX+SI]
		  case 0x01: case 0x09: /*SEG=SEG_DS;*/ break; // Mode 1:[BX+DI]
		  case 0x02: case 0x0A: SEG=SEG_SS; break; // Mode 2:[BP+SI]
		  case 0x03: case 0x0B: SEG=SEG_SS; break; // Mode 3:[BP+DI]
		  case 0x04: case 0x0C: /*SEG=SEG_DS;*/ break; // Mode 4:[SI]
		  case 0x05: case 0x0D: /*SEG=SEG_DS;*/ break; // Mode 5:[DI]
		  case 0x06: case 0x0E: // Mode 6: [BP+XX/XXXX] | [XX]
		  {
				  if(Extension==0) // 0x00-0x3F only! has special [XXXX]
				  {
					  /*SEG=SEG_DS;*/
                      SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
					  (*(*index))+=2; // read 2 bytes
				  }
				  else{ // 0x50-0xBF has [BP+]

					  SEG=SEG_SS; // SS Segment
				  }
		  }
		  break;
          
		  case 0x07: case 0x0F: /*SEG=SEG_DS;*/ break; // Mode 7: [BX]
		}

		// Choose used extension 
		// And Decode properly the menemonic
		switch(Extension)
		{
			case 0: // No extension of bytes to RegMem (except mode 6)
			{
				SwapDword((BYTE*)(*Opcode+pos),&dwOp,&dwMem);
				SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);

                if(((wOp&0x00FF)&0x0F)==0x06) // 0x00-0x3F with mode 6 only!
				{	                    
					(*Disasm)->OpcodeSize=4;
				}
				else{ // other modes                    
					(*Disasm)->OpcodeSize=2;
				}
			}
			break;

			case 1: // 1 Byte Extension to regMem
			{
                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
				FOpcode=wOp&0x00FF;
				
				if(FOpcode>0x7F) // check for signed numbers
				{
					FOpcode = 0x100-FOpcode; // -XX
				}
				++(*(*index)); // 1 byte read
				(*Disasm)->OpcodeSize=3;
			}
			break;
			
			case 2: // 2 Bytes Extension to RegMem
			{
                SwapDword((BYTE*)(*Opcode+pos),&dwOp,&dwMem);
                SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
				(*Disasm)->OpcodeSize=4;
				(*(*index))+=2; // we read 2 bytes
			}
			break;
		}

		// Switch Direction Mode.
		switch(Bit_d)
		{
			case 0: // (->)
			{
				switch(Op)// Check for all Cases
				{
                    case 0x6B:
                    {
                        // We check Extension because there is a diff
						// Reading position of bytes depend on the extension
						// 1 = read byte, 3rd position
						// 2 = read dword, 6th position
						
						if(Extension==1) // read 1 byte at 3rd position
						{
                            SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
							FOpcode=wOp&0x00FF;
						}
						else{ 
                            if(Extension==2) //read byte at 7th position (dword read before)
                            {   
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
                            else
                            { // Extension==0
                                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
						}
                        
						if(FOpcode>0x7F) // check for signed numbers!!
                        {
                            FOpcode = 0x100-FOpcode; // -XX (Signed)
                        }
						

						(*(*index))++;
						(*Disasm)->OpcodeSize++;
                    }
                    break;

                    case 0x81: case 0xC7: case 0x69:
					{
                        // Get Extensions!
                        //================    
                        
                        if(Extension==0)
                        {
                            if(PrefixReg==0)
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                            }
                        }
                        else if(Extension==1)
                        {
                            if(PrefixReg==0)
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                            }
                        }
                        else if(Extension==2)
                        {     
                            if(PrefixReg==0)
                            {                            
                                SwapDword((BYTE*)(*Opcode+pos+4),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                            }
                        }
                        
                        (*(*index))+=4;
                        (*Disasm)->OpcodeSize+=4;
					}
					break;
					
                    case 0x80:case 0x82: case 0x83: case 0xC6:
					{
						// We check Extension because there is a diff
						// Reading position of bytes depend on the extension
						// 1 = read byte, 3rd position
						// 2 = read dword, 6th position
						
						if(Extension==1) // read 1 byte at 3rd position
						{
                            SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                            FOpcode=wOp&0x00FF;
						}
						else{ 
                            if(Extension==2) //read byte at 7th position (dword read before)
                            {                            
                                SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
                            else
                            { // Extension==0
                                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
						}
						
                        if(Op==0x82 || Op==0x83)
						  if(FOpcode>0x7F) // check for signed numbers
                          {
							 FOpcode = 0x100-FOpcode; // -XX (Negative the Number)
                          }
												
						(*(*index))++;
						(*Disasm)->OpcodeSize++;
					}
					break;
					                    
                    case 0xF6:
                    {
                        // We check Extension because there is a diff
						// Reading position of bytes depend on the extension
						// 1 = read byte, 3rd position
						// 2 = read dword, 6th position
						
                        if(Extension==1) // read 1 byte at 3rd position
                        {
                            SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                            FOpcode=wOp&0x00FF;
                        }
                        else{ 
                            if(Extension==2) //read byte at 7th position (dword read before)
                            {                            
                                SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
                            else
                            { // Extension==0
                                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;
                            }
                        }
                        
                        if(reg1==0 || reg1==1)
                        {                        
						   (*(*index))++;
						   (*Disasm)->OpcodeSize++;
                        }
                    }
                    break;

                    case 0xF7:
                    {                        
                        // Get Extensions!
                        //================    
                        if(reg1==0 || reg1==1)
                        {                        
                            if(Extension==0)
                            {
                                if(PrefixReg==0)
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                                }
                            }
                            else if(Extension==1)
                            {
                                if(PrefixReg==0)
                                {                                
                                    SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                                    
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                                }
                            }
                            else if(Extension==2)
                            {         
                                if(PrefixReg==0)
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+4),&dwOp,&dwMem);
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                                }
                            }

							(*(*index))+=4;
                            (*Disasm)->OpcodeSize+=4;
                        }
                    }
                    break;
				}
			}
			break;

			case 1: // (<-) Direction (Bit_D)
			{
                // Check Used Opcode Set
                switch(Op)
                {
                    // Mixed Bit Rotation Instructions (rol/ror/shl..)
                    case 0xC0: case 0xC1:
                    {
                        // Check Extension
                        switch(Extension)
                        {
                            case 0: // No Extension
                            {
                                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;

                            case 1: // 1 byte Extension (Displacement)
                            {
                                SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;

                            case 2: // 2 Bytes Extension (Displacement)
                            {
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                                FOpcode=wOp&0x00FF;

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;
                        }                        
                    }
                    break;
                }
			}
			break;
		}

		++(*(*index)); // add 1 byte to index
		// no need to continue!! exit the function and proeed with decoding next bytes.
		return;
	}

    // ===================================================//
    //                NO SIB Being used!                  //
    // ===================================================//
	if(SIB!=SIB_EX) // NO SIB extension (i.e: 0x0001 = add byte ptr [ecx], al)
	{
		reg1=((BYTE)(*(*Opcode+pos+1))&0x07); // get register (we have only one)
		reg2=(((BYTE)(*(*Opcode+pos+1))&0x38)>>3);
        
		switch(Extension) // Check what extension we have (None/Byte/Dword)
		{
			case 00: // no extention to regMem
			{
				if(reg1==REG_EBP) // cannot display EBP as memoryReg, use DWORD mem location
				{
                    SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                    SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);

					(*Disasm)->OpcodeSize=6;
					(*(*index))+=5;
				}
				else{
                    SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);

					++(*(*index)); // only 1 byte read
					(*Disasm)->OpcodeSize=2; // total used opcodes
				}
			}
			break;
			
			case 01: // 1 btye extention to regMem
			{
                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);				

                FOpcode=wOp&0xFF; // get lower part of word.

				if(FOpcode>0x7F) // check for signed numbers
				{
					FOpcode = 0x100-FOpcode; // -XX
				}
				
				if(reg1==REG_EBP && PrefixSeg==0)
					SEG=SEG_SS;
				
				(*(*index))+=2; // x + 1 byte(s) read
				(*Disasm)->OpcodeSize=3; // total used opcodes
			}
			break;
			
			case 02: // 4 btye extention to regMem
			{
				// if ebp and there is no prefix 0x67, use SS segment
				if(reg1==REG_EBP && PrefixSeg==0)
					SEG=SEG_SS;

                SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
				SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);                

				(*(*index))+=5; // x + 1 + 4 byte(s) read
				(*Disasm)->OpcodeSize=6; // total used opcodes
			}
			break;
			//case 02:break;
		}

		switch(Bit_d)
		{
			case 0: // (->) Direction
			{
				switch(Op)// Check for all Cases Availble
				{

                    case 0x6B:
                    {
                        // We check Extension because there is a diff
						// Reading position of bytes depend on the extension
						// 1 = read byte, 3rd position
						// 2 = read dword, 6th position

						if(Extension==1) // read 1 byte at 3rd position
						{
							FOpcode=(BYTE)(*(*Opcode+pos+3));
						}
						else
                        { 
							if(Extension==2) // read byte at 7th position (dword read before)
                            {
								FOpcode=(BYTE)(*(*Opcode+pos+6));
							}
                            else if(Extension==0)
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+2));
                            }

                        }
                        
                        if(FOpcode>0x7F) // check for signed numbers!!
                        {
                            FOpcode = 0x100-FOpcode; // -XX
                        }
                        						
						(*(*index))++;
						(*Disasm)->OpcodeSize++;
                    }
                    break;


                    case 0x81: case 0xC7: case 0x69:// Opcode 0x81/0xC7/0x69
					{
                        // Get Extensions!
                        //================
						if(Extension==1) // 1 byte extersion
						{	
                            if(PrefixReg==0)
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                            }
						}
						else 
                            if(Extension==2) // 4 bytes Extensions
                            {  
                                if(PrefixReg==0)
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+6),&dwOp,&dwMem);
                                }
                                else //0x66 prefix
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+6),&wOp,&wMem);
                                }
                            }
                            else{ // No Extension!

                                if(PrefixReg==0)
                                {   
                                    if(reg1==REG_EBP)
                                    {
                                        SwapDword((BYTE*)(*Opcode+pos+6),&dwOp,&dwMem);
                                    }
                                    else{
                                        SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                                    }
                                }
                                else // 0x66 prefix
                                {
                                    if(reg1==REG_EBP)
                                    {
                                        SwapWord((BYTE*)(*Opcode+pos+6),&wOp,&wMem);
                                    }
                                    else
                                    {   
                                        SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                                    }
                                }
                        }

                        if(PrefixReg==0)
                        {                        
                            (*(*index))+=4;
                            (*Disasm)->OpcodeSize+=4;
                        }
                        else
                        {
                            (*(*index))+=2;
                            (*Disasm)->OpcodeSize+=2;
                        }

					}
					break;

                    case 0x80:case 0x82: case 0x83: case 0xC6:
					{
						// We check Extension because there is a diff
						// Reading position of bytes depend on the extension
						// 1 = read byte, 3rd position
						// 2 = read dword, 6th position

						if(Extension==1) // read 1 byte at 3rd position
						{
							FOpcode=(BYTE)(*(*Opcode+pos+3));
						}
						else
                        { 
							if(Extension==2) // read byte at 7th position (dword read before)
                            {
								FOpcode=(BYTE)(*(*Opcode+pos+6));
							}
                            else if(Extension==0)
                            {
                                if(reg1==REG_EBP)
                                   FOpcode=(BYTE)(*(*Opcode+pos+6));
                                else
                                   FOpcode=(BYTE)(*(*Opcode+pos+2));
                            }

                        }
						
                        // Opcodes with signed number
                        if(Op==0x82 || Op==0x83)
						  if(FOpcode>0x7F) // check for signed numbers
                          {
							 FOpcode = 0x100-FOpcode; // -XX (Negative the Number)
                          } 
												
						(*(*index))++;
						(*Disasm)->OpcodeSize++;
					}
					break;

                    case 0xF6: // MIXED Instructions (MUL,DIV,NOT...)
                    {
                       // We check Extension because there is a diff
                       // Reading position of bytes depend on the extension
                       // 1 = read byte, 3rd position
                       // 2 = read dword, 6th position
                            
                       if(Extension==1) // read 1 byte at 3rd position
                       {
                           if(reg2==0 || reg2==1) // TEST Only
                           {                            
                               FOpcode=(BYTE)(*(*Opcode+pos+3));
                           }
                       }
                       else
                       { 
                           if(Extension==2) // read byte at 7th position (dword read before)
                           {
                               if(reg2==0 || reg2==1) // TEST Only
                               {                                
                                   FOpcode=(BYTE)(*(*Opcode+pos+6));
                               }
                           }
                           else if(Extension==0)
                           {
                               if(reg2==0 || reg2==1) // TEST Only
                               {                                
                                   FOpcode=(BYTE)(*(*Opcode+pos+2));
                               }
                           }
                             
                       }
                            
                       if(reg2==0 || reg2==1) // TEST instruction
                       {
                           (*(*index))++;
                           (*Disasm)->OpcodeSize++;
                       }
                            
                    }
                    break;

                    case 0xF7:
                    {

                        //================//
                        // Get Extensions!//
                        //================//

                        if(reg2==0 || reg2==1){ // TEST Instruction
                        
						if(Extension==1) // 1 byte extersion
						{	
                            if(PrefixReg==0)
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                            }
						}
						else if(Extension==2) // 4 bytes Extensions
						{   
                            if(PrefixReg==0)
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+6),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+6),&wOp,&wMem);
                            }
                        }
                        else{ // No Extension!  (check ebp)

                            if(PrefixReg==0)
                            {   
                                if(reg1==REG_EBP)
                                {
                                    SwapDword((BYTE*)(*Opcode+pos+6),&dwOp,&dwMem);
                                }
                                else
                                {                                
                                    SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                                }
                            }
                            else
                            {
                                if(reg1==REG_EBP)
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+6),&wOp,&wMem);
                                }
                                else 
                                {                                
                                    SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);
                                }
                            }
                        }

                        if(PrefixReg==0)
                        {                        
                            (*(*index))+=4;
                            (*Disasm)->OpcodeSize+=4;
                        }
                        else
                        {
                            (*(*index))+=2;
                            (*Disasm)->OpcodeSize+=2;
                        }
                      }
                    }
                    break;

				}				
			}
			break;
			
			case 1: // (<-) Direction of decoding
			{
                switch(Op)
                {
                    // Mixed Bit Rotation Instructions (rol/ror/shl..)
                    case 0xC0:case 0xC1:
                    {
						switch(Extension) {
							case 0:
							{
								if(reg1==REG_EBP)
								{
								  FOpcode=(BYTE)(*(*Opcode+pos+6));
								}
								else
									FOpcode=(BYTE)(*(*Opcode+pos+2)); 
							}
							break;
							case 1: FOpcode=(BYTE)(*(*Opcode+pos+3)); break;
							case 2: FOpcode=(BYTE)(*(*Opcode+pos+6)); break;
						}
						
                        (*(*index))++;
                        (*Disasm)->OpcodeSize++;
                    }
                    break;                    
                }
			}
			break;
		}
        
        return;
	}
    // ===================================================//
    //                 SIB is being used!                 //
    // ===================================================//
	else if(SIB==SIB_EX) // Found SIB, lets strip the extensions
	{
		reg1=((BYTE)(*(*Opcode+pos+2))&0x38)>>3;  // Register A
		reg2=((BYTE)(*(*Opcode+pos+2))&0x07);     // Register B
		SCALE=((BYTE)(*(*Opcode+pos+2))&0xC0)>>6; // Scale size (0,2,4,8)

        /* 
           Check for valid/invalid pop instruction,
           pop insteruction must have reg bit 000
           pop code/ModRM:
           
           Code Block: 1000 1111
           Mod/RM: oo000mmm 
           oo - Mod
           000 - Must be 0
           mmm - <reg>
        
        */

		switch(Extension) // +/+00/+00000000
		{
			case 00: // No extension of bytes
			{
                if(reg1==REG_ESP && reg2!=REG_EBP)
                {
                    if(reg2==REG_ESP) SEG=SEG_SS; // IF ESP is being used, User SS Segment Overridr
                    SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);

					(*(*index))+=2; //2 byte read				
					(*Disasm)->OpcodeSize=3; // total used opcodes
                }
				else if(reg2!=REG_EBP) // No EBP in RegMem
				{
                    if(reg2==REG_ESP) SEG=SEG_SS; // IF ESP is being used, User SS Segment Overridr
                    SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);

					(*(*index))+=2; //2 byte read				
					(*Disasm)->OpcodeSize=3; // total used opcodes
				}
				else if(reg2==REG_EBP) // Replace EBP with Dword Number
				{
					// get 4 bytes extensions for memReg addon
					// insted of Normal Registers

					// Format Opcodes (HEX)
                    SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                    SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);

                    Extension=2; // OverRide Extension (?????), Check toDo.txt
					(*(*index))+=6; //6 byte read				
					(*Disasm)->OpcodeSize=7; // total used opcodes
				}
			}
			break;

			case 01: // 1 byte extension
			{				
				FOpcode=(BYTE)(*(*Opcode+pos+3));
				if(FOpcode>0x7F) // check for signed numbers!!
				{
					FOpcode = 0x100-FOpcode; // -XX
				}
				
				if(/*reg2==REG_EBP ||*/ reg1==REG_ESP) // no ESP in [Mem]
                {                
					SEG=SEG_SS;
                }

				(*(*index))+=3; // x + 3 byte(s) read				
				
                SwapDword((BYTE*)(*Opcode+pos),&dwOp,&dwMem);

				(*Disasm)->OpcodeSize=4; // total used opcodes
			}
			break;

			case 02:// Dword extension
			{
                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                
                if(reg1!=REG_ESP) 
                {
                    if(reg2==REG_EBP || reg2==REG_ESP)
                        SEG=SEG_SS;                    
                }
                else
                {// ESP Must not be as Index, Code = 100b
                    
                    if(reg2==REG_ESP)
                        SEG=SEG_SS;
                }

				(*(*index))+=6; // x + 3 byte(s) read	
                (*Disasm)->OpcodeSize=7; // total used opcodes
            }
			break;
		}

		// Finish up the opcode with position of target register
		switch(Bit_d)
		{
			case 0: // (->) Direction
			{
				switch(Op)// Check for all Cases
				{
                    case 0x6B:
                    {
                        // We check Extension because there is a diff
                        // Reading position of bytes depend on the extension
                        // 1 = read byte, 3rd position
                        // 2 = read dword, 6th position
                        
                        if(Extension==1) // read 1 byte at 3rd position
                        {
                            FOpcode=(BYTE)(*(*Opcode+pos+3));
                        }
                        else
                        { 
                            if(Extension==2) // read byte at 7th position (dword read before)
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+7));
                            }
                            else if(Extension==0)
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+2));
                            }
                            
                        }
                        
                        if(FOpcode>0x7F) // check for signed numbers!!
                        {
                            FOpcode = 0x100-FOpcode; // -XX
                        }
                        
						
						BYTE Level=(BYTE)(*(*Opcode+pos+1));
						if( (Level>=0x00 && Level<=0x07) || (Level>=0x40 && Level<=0x47) || (Level>=0x80 && Level<=0x87) )
							reg2=0;

						if( (Level>=0x08 && Level<=0x0F) || (Level>=0x48 && Level<=0x4F) || (Level>=0x88 && Level<=0x8F) )
							reg2=1;
						
						if( (Level>=0x10 && Level<=0x17) || (Level>=0x50 && Level<=0x57) || (Level>=0x90 && Level<=0x97) )
							reg2=2;

						if( (Level>=0x18 && Level<=0x1F) || (Level>=0x58 && Level<=0x5F) || (Level>=0x98 && Level<=0x9F) )
							reg2=3;

						if( (Level>=0x20 && Level<=0x27) || (Level>=0x60 && Level<=0x67) || (Level>=0xA0 && Level<=0xA7) )
							reg2=4;
						
						if( (Level>=0x28 && Level<=0x2F) || (Level>=0x68 && Level<=0x6F) || (Level>=0xA8 && Level<=0xAF) )
							reg2=5;

						if( (Level>=0x30 && Level<=0x37) || (Level>=0x70 && Level<=0x77) || (Level>=0xB0 && Level<=0xB7) )
							reg2=6;
						
						if( (Level>=0x38 && Level<=0x3F) || (Level>=0x78 && Level<=0x7F) || (Level>=0xB8 && Level<=0xBF) )
							reg2=7;
                        
                        (*(*index))++;
						(*Disasm)->OpcodeSize++;
                    }
                    break;

                    case 0x81: case 0xC7: case 0x69: // Opcodes 0x81/0xC7/0x69
                    {
                        // Get Extensions!
                        //================
                        if(Extension==1) // 1 byte extersion
                        {
                            if(PrefixReg==0) // No Reg Prefix
                            {
                                SwapDword((BYTE*)(*Opcode+pos+4),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                            }
                        }
                        else if(Extension==2) // 4 bytes Extensions
                        {    
                            if(PrefixReg==0) // No Reg Prefix
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+7),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+7),&wOp,&wMem);
                            }
                        }
                        else{ // No Extension!
                            
                            if(PrefixReg==0) // No Reg Prefix
                            {   
                                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                            }
                            else
                            {
                                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                            }
                        }
                        
                        if(Op==0xC7)
                        {
                            /* 
                                Instruction rule: Mem,Imm ->  1100011woo000mmm,imm
                                Code Block: 1100011
                                w = Reg Size
                                oo - Mod
                                000 - Must be!
                                mmm - Reg/Mem
                                imm - Immidiant (קבוע)
                            */
                            
                            reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3; // Check for valid opcode, result must be 0                            
                        }
                        else
                        {
                            if (Op==0x69) // IMUL REG,MEM,IIM
                            {
                                reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3; // get register
                            }
                        }
                        

						if(PrefixReg==0) // No regPrefix
                        {                        
                            (*(*index))+=4;
                            (*Disasm)->OpcodeSize+=4;
                        }
                        else
                        {
                            (*(*index))+=2;
                            (*Disasm)->OpcodeSize+=2;
                        }
                    }
					break;

                    case 0x80:case 0x82: case 0x83: case 0xC6:
					{
						if(Extension==1)// read 1 byte at 3rd position
						{
							FOpcode=(BYTE)(*(*Opcode+pos+4));
						}
						else
						{ 
                            if(Extension==2)
                            {
                                // read byte at 7th position (dword read before)
                                FOpcode=(BYTE)(*(*Opcode+pos+7));
							}
                            else if (Extension==0)
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+3));
                            }
						}
                        
						
                        if(Op==0x82 || Op==0x83)
						  if(FOpcode>0x7F) // check for signed numbers
                          {
							 FOpcode = 0x100-FOpcode; // -XX (Negative the Number)
                          }
						
                        // Code Block of C6 is Mov instruction
                        if(Op==0xC6)
                        {
                            /* 
                                Instruction rule: Mem,Imm ->  1100011woo000mmm,imm
                                Code Block: 1100011
                                w = Reg Size
                                oo - Mod
                                000 - Must be!
                                mmm - Reg/Mem
                                imm - Immidiant (קבוע)
                            */

                            // Check for valid intruction, reg1 must be 000 to be valid
                            reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3; 
                        }
						
						(*(*index))++;
						(*Disasm)->OpcodeSize++;
					}
					break;

                    case 0xF6:
                    {
                        // strip Instruction Bits (1111011woo[000]mmm)
                        reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3;

                        if(Extension==1)// read 1 byte at 3rd position
						{
                            if(reg1==0 || reg1==1) // check bites: TEST 
                            {                            
                                FOpcode=(BYTE)(*(*Opcode+pos+3));
                            }
						}
						else
						{ 
                            if(Extension==2)
                            {
                                if(reg1==0 || reg1==1) // check bites: TEST 
                                {                                
                                    // read byte at 7th position (dword read before)
                                    FOpcode=(BYTE)(*(*Opcode+pos+7));
                                }
							}
                            else if (Extension==0)
                            {
                                if(reg1==0 || reg1==1) // check bites: TEST 
                                {                                
                                    FOpcode=(BYTE)(*(*Opcode+pos+2));
                                }
                            }
						}

                        if(reg1==0 || reg1==1) // TEST
                        {                        
                            (*(*index))++;
						    (*Disasm)->OpcodeSize++;
                        }
                    }
                    break;

                    case 0xF7:
                    {
                        // Get Instruction
                        reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3;

                        //================//
                        // Get Extensions!//
                        //================//
                        
                        if(reg1==0 || reg1==1){ // TEST Instruction
                            
                            if(Extension==1) // 1 byte extersion
                            {	
                                if(PrefixReg==0)  // no 0x66 prefix
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+4),&dwOp,&dwMem);
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+4),&wOp,&wMem);
                                }
                            }
                            else if(Extension==2) // 4 bytes Extensions
                            {    
                                if(PrefixReg==0)
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+7),&dwOp,&dwMem);
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+7),&wOp,&wMem);
                                }
                            }
                            else{ // No Extension!
                                
                                if(PrefixReg==0)
                                {   
                                    SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                                }
                                else
                                {
                                    SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
                                }
                            }
                            
                            if(PrefixReg==0) // No Reg prefix
                            {                            
                                (*(*index))+=4;
                                (*Disasm)->OpcodeSize+=4;
                            }
                            else
                            {
                                (*(*index))+=2;
                                (*Disasm)->OpcodeSize+=2;
                            }
                        }
                    }
                    break;                    
				}
			}
			break;

			case 1: // (<-) Direction
			{
                switch(Op) 
                {                    
                    // Mixed Bit Rotation Instructions (rol/ror/shl..)
                    case 0xC0:case 0xC1:
                    {
                        switch(Extension)
                        {
                            case 0:
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+3));

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;

                            case 1:
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+4));

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;

                            case 2:
                            {
                                FOpcode=(BYTE)(*(*Opcode+pos+7));

								(*(*index))++;
                                (*Disasm)->OpcodeSize++;
                            }
                            break;
                        }                        
                    }
                    break;
                }
			}
			break;
		}		
	}	
}


// ========================================
// ====== Convert Hex String to DWORD =====
// ========================================
DWORD StringToDword(char *Text)
{
	/*
	this function will convert and return
	an Hexadecimel String into a real
	DWORD hex number using assembly directive.
    */
	
    DWORD AsmDwordNum=PtrToUlong(Text);
    DWORD DwordNum=0;
	
	_asm{
		PUSHAD
			PUSHF
			XOR ECX,ECX
			XOR EAX,EAX
			XOR EDI,EDI
			MOV EDI,8H
			MOV ESI,AsmDwordNum
_start:
		MOV CL,[ESI]
			CMP CL,30H
			JL _lower
			CMP CL,39H
			JG _upper
			SUB CL,30H
			JMP _jmp1
_upper:
		SUB CL,37H
			JMP _jmp1
_lower:
_jmp1:
		ADD EAX,ECX
			CMP EDI,1
			JZ _out
			SHL EAX,4H
_out:
		INC ESI
			DEC EDI
			JNZ _start
			MOV DwordNum,EAX
			POPF
			POPAD
	}
	
	return DwordNum;
}

// =======================================
// ====== Convert Hex String to WORD =====
// =======================================
WORD StringToWord(char *Text)
{
	/*
	this function will convert and return
	an Hexadecimel String into a real
	WORD hex number using assembly directive.
    */
	
    DWORD AsmDwordNum=PtrToUlong(Text);
    WORD DwordNum=0;
	
	_asm{
		PUSHAD
			PUSHF
			XOR ECX,ECX
			XOR EAX,EAX
			XOR EDI,EDI
			MOV EDI,4H
			MOV ESI,AsmDwordNum
_start:
		MOV CL,[ESI]
			CMP CL,30H
			JL _lower
			CMP CL,39H
			JG _upper
			SUB CL,30H
			JMP _jmp1
_upper:
		SUB CL,37H
			JMP _jmp1
_lower:
_jmp1:
		ADD EAX,ECX
			CMP EDI,1
			JZ _out
			SHL EAX,4H
_out:
		INC ESI
			DEC EDI
			JNZ _start
			MOV DwordNum,AX
			POPF
			POPAD
	}
	
	return DwordNum;
}


int GetNewInstruction(BYTE Op)
{
    // return values:
    // Found = 0 -> big set instruction
    // Found = 1 -> 1 byte Instruction
    // Found = 2 -> Jump Instruction
    
    int Found=1;
    
    switch(Op)
    {
        // 1 BYTE INSTRUCTIONS
        case 0x05: break;
        case 0x06: break;
        case 0x07: break;
        case 0x08: break;
        case 0x09: break;
        case 0x0B: break;
        case 0x0E: break;       
        case 0x30: break;
        case 0x31: break;
        case 0x32: break;
        case 0x33: break;
        case 0x34: break;
        case 0x35: break;
        case 0x77: break;        
        case 0xA0: break;
        case 0xA1: break;
        case 0xA2: break;
        case 0xA8: break;
        case 0xA9: break;
        case 0xAA: break;
        
        // BSWAP <REG>
        case 0xC8: case 0xC9: case 0xCA: case 0xCB: 
        case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        {
        }
        break;        

        // Invalid instructions, but have s valid 0xC0
        case 0x20: case 0x21:case 0x22:
        case 0x23: case 0x50: case 0xBA:case 0x71: 
        case 0x72:
        {
            Found=3;
        }
        break;

        // INVALID INSTRUCTIONS!!
        case 0x0F: case 0x19: case 0x1A: case 0x1B: 
        case 0x1C: case 0x1D: case 0x1E: case 0x04: 
        case 0x1F: case 0x0A: case 0x0C: case 0x24:
        case 0x2B: case 0x36: case 0x37: case 0x25:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        case 0x5A: case 0x5B: case 0x6C: case 0x6D:
        case 0x73: case 0x78: case 0x26: case 0x27:
        case 0x79: case 0x7A: case 0x7B: case 0x7C:
        case 0x7D: case 0xA6: case 0xA7: case 0xB8:
        case 0xB9: case 0xC3: case 0xD0:
        case 0xD4: case 0xD6: case 0xE6: case 0xF0:
        case 0xF4: case 0xFB: case 0xFF:          
        {
        }
        break;

        // JUMPS [JXX]
        case 0x80:case 0x81:case 0x82:case 0x83:
        case 0x84:case 0x85:case 0x86:case 0x87:
        case 0x88:case 0x89:case 0x8A:case 0x8B:
        case 0x8C:case 0x8D:case 0x8E:case 0x8F:         
        {
            Found=2;
        }
        break;

		// 0F C7 XX [XX- has valid 0x08-0x0F]
		case 0xC7:
		{
			Found=4;
		}
		break;
        
        default: Found=0; break;
    }

    return Found;
}

//=====================================================================================//
//                     Decode MMX / 3DNow! / SSE / SSE2 Functions                      //
//=====================================================================================//

void Mod_11_RM_EX(BYTE d, BYTE w, char **Opcode, DISASSEMBLY **Disasm, bool PrefixReg, BYTE Op, DWORD **index, bool RepPrefix)
{
   /* 
       Function Mod_11_RM Checks whatever we have
	   Both bit d (direction) and bit w (full/partial size).
	 
       There are 4 states:
	   00 - d=0 / w=0 ; direction -> (ie: DH->DL),   partial size (AL,DH,BL..)
	   01 - d=0 / w=1 ; direction -> (ie: EDX->EAX), partial size (EAX,EBP,EDI..)
	   10 - d=1 / w=0 ; direction <- (ie: DH<-DL),   partial size (AL,DH,BL..)
	   11 - d=1 / w=1 ; direction <- (ie: EDX<-EAX), partial size (EAX,EBP,EDI..)
	
       Also deals with harder opcodes which have diffrent
       Addresing type.
    */
    
	int RM,IndexAdd=1,m_OpcodeSize=2,Pos; // Register(s) Pointer
	WORD wOp,wMem;
    BYTE reg1=0,reg2=0,m_Opcode=0,REG;
    
    Pos=(*(*index)); // Current Position
    
    m_Opcode = (BYTE)(*(*Opcode+Pos+1));// Decode registers from second byte
    
    // Strip Used Instructions / Used Segment
    REG=m_Opcode; 
    REG>>=3;
	REG&=0x07;

    // (->) / reg8
    if(d==0 && w==0)
    {    
        RM=REG8;
        reg1=(m_Opcode&0x07);
        reg2=(m_Opcode&0x38)>>3;
    }
    
    // (->) / reg32
    if(d==0 && w==1)
    {    
        RM=REG32;
        if(PrefixReg==1)
            RM=REG16; // (->) / reg16 (RegPerfix is being used)
        
        reg1=(m_Opcode&0x07);
        reg2=(m_Opcode&0x38)>>3;
    }
    
    // (<-) / reg8
    if(d==1 && w==0)
    {    
        RM=REG8;
        reg2=(m_Opcode&0x07);
        reg1=(m_Opcode&0x38)>>3;
    }
    
    // (<-) / reg32
    if(d==1 && w==1)
    {    
        RM=REG32;
        if(PrefixReg==1)
            RM=REG16; // (<-) / reg16
        
        reg2=(m_Opcode&0x07);
        reg1=(m_Opcode&0x38)>>3;
    }

    switch(Op)
    {
        case 0x00: 
        {
            RM=REG16; // FORCE 16BIT
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x01: 
        {
            RM=REG32; // DEFAULT 32Bit

            if(REG>=4 && REG<=6) // USES 32bit
                RM=REG16;

            if(REG==7) // USES 8BIT
                RM=REG8;

            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x02: // LAR
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x03: // LSL
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x10: // MOVUPS
        {
           if(RepPrefix)
           {
               m_OpcodeSize++;
           }
           
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x11: // MOVUPS
        {
           if(RepPrefix)
           {
                m_OpcodeSize++;
           }

           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x12: // MOVHLPS
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x13: // MOVLPS
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x14: // UNPCKLPS
        {
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x15: // UNPCKHPS
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x16: // MOVLHPS
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x17: // MOVHPS
        {
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x18:
        {
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x28: // MOVAPS
        {
           SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x29: // MOVAPS
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x2A: // CVTPI2PS
        {
          if(RepPrefix==1)
          {
              m_OpcodeSize++;
          }
          
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break; 

        case 0x2C: case 0x2D: // CVTTPS2PI, CVTPS2PI
        {
            if(RepPrefix==1)
            {
                m_OpcodeSize++;
            }

            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x2E: case 0x2F: // UCOMISS, COMISS
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x40:case 0x41:case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:
        case 0x48:case 0x49:case 0x4A:case 0x4B:case 0x4C:case 0x4D:case 0x4E:case 0x4F:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:
        case 0x58:case 0x59:case 0x5C:case 0x5D:case 0x5E:case 0x5F:
        {
            if(RepPrefix==1) // Rep Prefix is being used
            {
                m_OpcodeSize++;
            }
            
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break; // MIX
        
        case 0x60:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:
        case 0x68:case 0x69:case 0x6A:case 0x6B:case 0x6E:case 0x6F:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x70:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0x74:case 0x75:case 0x76: // MIX
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x7E:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x7F:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x90:case 0x91:case 0x92:case 0x93:case 0x94:case 0x95:case 0x96:case 0x97:
        case 0x98:case 0x99:case 0x9A:case 0x9B:case 0x9C:case 0x9D:case 0x9E:case 0x9F:
        {
          RM=REG8; // FORCE 8BIT
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break; // MIX

        case 0xA3: case 0xAB:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xA4: case 0xAC:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xA5: case 0xAD:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xAE:
        {            
            if(REG>3)// Check for Invalid
            {
                m_Opcode=(BYTE)(*(*Opcode+Pos+1));
            }
        }
        break;

        case 0xAF:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xB0: case 0xB1: case 0xB3: case 0xBB:
        {
          if((Op&0x0F)==0)
              RM=REG8;

          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xB2:case 0xB4:case 0xB5:
        case 0xB6:case 0xB7:case 0xBC: 
        case 0xBD:case 0xBE:case 0xBF:
        {
          BYTE reg=Op&0x0F;
          int RM2=REG32; // default
          
          if(reg==0x06 || reg==0x0E) 
              RM2=REG8;
          
          if(reg==0x07 || reg==0x0F)
              RM2=REG16;
          
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xC0: case 0xC1:
        {
          if(Op==0xC0)
              RM=REG8;

          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xC2:
        {
            if((BYTE)(*(*Opcode+Pos+2))<8) // Instructions here
            {
                if(RepPrefix==1)
                {
                    m_OpcodeSize++;
                }
            }
            else
            {
                if(RepPrefix==1)
                {
                    m_OpcodeSize++;
                }
            }
            
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xC4:
        {
            RM=REG16;
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xC5:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xC6:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xD7: 
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;
        case 0xD1:case 0xD2:case 0xD3:case 0xD5:case 0xD8:case 0xDF:
        case 0xD9:case 0xDA:case 0xDB:case 0xDC:case 0xDD:case 0xDE:                    
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xE0:case 0xE1:case 0xE2:case 0xE3:
        case 0xE4:case 0xE5:case 0xE8:case 0xE9:
        case 0xEA:case 0xEB:case 0xEC:case 0xED:
        case 0xEE:case 0xEF:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xE7:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0xF1:case 0xF2:case 0xF3:case 0xF5:case 0xF6:
        case 0xF7:case 0xF8:case 0xF9:case 0xFA:case 0xFC:
        case 0xFD:case 0xFE:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x0D:
        {
            BYTE NextByte = (BYTE)(*(*Opcode+Pos+1));
            
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x20:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x21:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x22:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x23:
        {
          SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x50:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);
        }
        break;

        case 0x71:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0x72:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;

        case 0xBA:
        {
            SwapWord((BYTE*)(*Opcode+Pos),&wOp,&wMem);

			(*(*index))++;
            m_OpcodeSize++;
        }
        break;
        
    }
    
    (*Disasm)->OpcodeSize=m_OpcodeSize;
    (*(*index))++;
}




void Mod_RM_SIB_EX(
                    DISASSEMBLY **Disasm,
                    char **Opcode, int pos, 
                    bool AddrPrefix,
                    int SEG,
                    DWORD **index,
                    BYTE Op,
                    bool PrefixReg,
                    bool PrefixSeg,
                    bool PrefixAddr,
                    BYTE Bit_d,
                    BYTE Bit_w,
                    bool RepPrefix
                )
{
   /*
        This Function will resolve BigSet menemonics: 
        Of MMX,3DNow! and New Set Instructions.
    */

	// Set Defaults    
    DWORD dwOp,dwMem;
    int RM=REG8,SCALE=0,SIB,ADDRM=REG32;
    WORD wOp,wMem;
	BYTE reg1=0,reg2=0,REG=0,Extension=0,FOpcode=0;

    // Get used Register
	// Get target register, example:
	// 1. add byte ptr [ecx], -> al <-
	// 2. add -> al <- ,byte ptr [ecx]
    REG=(BYTE)(*(*Opcode+pos+1)); 
	REG>>=3;
	REG&=0x07;

    //Displacement MOD (none|BYTE/WORD|DWORD)
	Extension=(BYTE)(*(*Opcode+pos+1))>>6;


    switch((BYTE)(*(*Opcode+pos)))
    {
      case 0x00:{Bit_d=0; Bit_w=1; } break; // WORD
      case 0x01:{
                 Bit_d=0; Bit_w=1;
                }
                break;
      case 0x02: case 0x03:{ Bit_d=1; Bit_w=1; }    break; // DWORD
      case 0x0D: { Bit_d=1; Bit_w=1; }			  break; // DWORD
	  case 0x10: { Bit_d=1; Bit_w=1; }              break; // DQWORD
      case 0x11: { Bit_d=0; Bit_w=1; }              break; // DQWORD
      case 0x12: case 0x16: { Bit_d=1; Bit_w=1;  }   break; // QWORD
      case 0x13: case 0x17: { Bit_d=0; Bit_w=1;  }   break; // QWORD
      case 0x14: case 0x15: { Bit_d=1; Bit_w=1;  }   break; // DQWORD
      case 0x18: { Bit_d=1; Bit_w=1;  }              break; // QWORD
      case 0x28: { Bit_d=1; Bit_w=1;  }              break; // DQWORD
      case 0x29: { Bit_d=0; Bit_w=1;  }              break; // DQWORD
      case 0x2A: case 0x2C: case 0x2D:case 0x2E: case 0x2F:                  
      {
          Bit_d=1; Bit_w=1;
      }
      break;      
      case 0x40:case 0x41:case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:
      case 0x48:case 0x49:case 0x4A:case 0x4B:case 0x4C:case 0x4D:case 0x4E:case 0x4F:
          { Bit_d=1; Bit_w=1; }  // DWORD
      break;

      case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:
      case 0x58:case 0x59:case 0x5C:case 0x5D:case 0x5E:case 0x5F:
          { Bit_d=1; Bit_w=1; } // DQWORD
      break;
      case 0x60:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:
      case 0x68:case 0x69:case 0x6A:case 0x6B:case 0x6E:case 0x6F:
          { Bit_d=1; Bit_w=1; }  // DWORD/QWORD
      break;
      case 0x70:case 0x74:case 0x75:case 0x76: { Bit_d=1; Bit_w=1; } break; // QWORD
      case 0x7E: { Bit_d=0; Bit_w=1; }  break;   // DWORD
      case 0x7F: { Bit_d=0; Bit_w=1; }  break;   // QWORD
      case 0x90:case 0x91:case 0x92:case 0x93:case 0x94:case 0x95:case 0x96:case 0x97:
      case 0x98:case 0x99:case 0x9A:case 0x9B:case 0x9C:case 0x9D:case 0x9E:case 0x9F:
          { Bit_d=1; Bit_w=1; }  // BYTE
      break;
      case 0xA3:case 0xA4:case 0xA5:case 0xAB:case 0xAC:case 0xAD:{ Bit_d=0; Bit_w=1; } break;//DWORD
      case 0xAE:
      { 
          Bit_d=1; Bit_w=1;
      } 
      break; //512Byte / DWORD  (FXSAVE)
      case 0xAF:            { Bit_d=1; Bit_w=1; }            break; // DWORD  (IMUL)
      case 0xB0:            { Bit_d=0; Bit_w=1; }            break; // BYTE   (CMPXCHG)
      case 0xB1: case 0xB3: case 0xBB:{ Bit_d=0; Bit_w=1; }  break; // DWORD  (CMPXCHG/BTC/BTR)
      case 0xB2: case 0xB4: case 0xB5:{ Bit_d=1; Bit_w=1; }  break; // FWORD  (LSS/LFS/LGS)
      case 0xB6: case 0xBE: { Bit_d=1; Bit_w=1; }            break; // BYTE   (MOVSX/MOVZX)
      case 0xB7: case 0xBF: { Bit_d=1; Bit_w=1; }            break; // WORD   (MOVSX/MOVZX)
      case 0xBC: case 0xBD: { Bit_d=1; Bit_w=1; }            break; // DWORD  (BSF/BSR)
      case 0xC0:            { Bit_d=0; Bit_w=1; }            break; // BYTE   (XADD)
      case 0xC1:            { Bit_d=0; Bit_w=1; }            break; // DWORD  (XADD)
      case 0xC2: case 0xC6: { Bit_d=1; Bit_w=1; }            break; // DQWORD (MIX)
      case 0xC4:            { Bit_d=1; Bit_w=1; }            break; // WORD   (MIX)
      case 0xC5: case 0xC7: { Bit_d=1; Bit_w=1; }            break; // QWORD  (MIX)
      
      case 0xD1:case 0xD2:case 0xD3:case 0xD5:case 0xD7:
      case 0xD8:case 0xD9:case 0xDA:case 0xDB:case 0xDC:
      case 0xDD:case 0xDE:case 0xDF:case 0xE0:case 0xE1:
      case 0xE2:case 0xE3:case 0xE4:case 0xE5:case 0xE8:
      case 0xE9:case 0xEA:case 0xEB:case 0xEC:case 0xED:
      case 0xEE:case 0xEF:
      case 0xF1:case 0xF2:case 0xF3:case 0xF5:case 0xF6:
      case 0xF7:case 0xF8:case 0xF9:case 0xFA:case 0xFC:
      case 0xFD:case 0xFE:
      {
          Bit_d=1; Bit_w=1; 
      }
      break;

      case 0xE7: Bit_d=0; Bit_w=1; break; // QWORD      
      
    }

    	// check for bit register size : 16bit/32bit
	if(Bit_w==1)
	{
	   RM=REG32; // 32bit registers set                   
	}
	
    if(PrefixReg==1) // Change 32bit Data Size to 16Bit
    {
        // All Opcodes with DWORD Data Size
        BYTE DOpcodes[35]={ 
                    0x02,0x03,0x2E,0x2F,0x6E,0x7E,
                    0xA3,0xA4,0xA5,0xAB,0xAC,0xAD,
                    0xAF,0xB1,0xB3,0xBB,0xBC,0xBD,
                    0xC1,0x40,0x41,0x42,0x43,0x44,
                    0x45,0x46,0x47,0x48,0x49,0x4A,
                    0x4B,0x4C,0x4D,0x4E,0x4F
        };

        for(int i=0;i<35;i++)
            if(Op==DOpcodes[i])
            {            
                RM=REG16; // 16bit registers
                break;
            }
    }
    

    // SCALE INDEX BASE
	SIB=(BYTE)(*(*Opcode+pos+1))&0x07; // Get SIB extension


    // ===================================================//
    //             AddrPrefix is being used!               //
    // ===================================================//
    
    if(PrefixAddr==1) // Prefix 0x67 is set, Change Segments/Addressing Modes to 16 bits
    {		
        FOpcode=((BYTE)(*(*Opcode+pos+1))&0x0F); // Get addressing Mode (8 types of mode)
        reg1=((BYTE)(*(*Opcode+pos+1))&0x38)>>3;
        
        // Choose Mode + Segment
        switch(FOpcode)
        {
            case 0x00: case 0x08: SEG=SEG_DS; break; // Mode 0:[BX+SI]
            case 0x01: case 0x09: SEG=SEG_DS; break; // Mode 1:[BX+DI]
            case 0x02: case 0x0A: SEG=SEG_SS; break; // Mode 2:[BP+SI]
            case 0x03: case 0x0B: SEG=SEG_SS; break; // Mode 3:[BP+DI]
            case 0x04: case 0x0C: SEG=SEG_DS; break; // Mode 4:[SI]
            case 0x05: case 0x0D: SEG=SEG_DS; break; // Mode 5:[DI]
            case 0x06: case 0x0E: // Mode 6: [BP+XX/XXXX] | [XX]
            {
                if(Extension==0) // 0x00-0x3F only! has special [XXXX]
                {
                    SEG=SEG_DS;
                    SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);

					(*(*index))+=2; // read 2 bytes
                }
                else{ // 0x50-0xBF has [BP+]
                    
                    SEG=SEG_SS; // SS Segment
                }
            }
            break;
            
            case 0x07: case 0x0F: SEG=SEG_DS; break; // Mode 7: [BX]
        }
        
        // Choose used extension 
        switch(Extension)
        {
            case 0: // No extension of bytes to RegMem (except mode 6)
            {
                SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);
                SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);

                if(((wOp&0x00FF)&0x0F)==0x06) // 0x00-0x3F with mode 6 only!
                {					
                    (*Disasm)->OpcodeSize=4;
                    FOpcode=(BYTE)(*(*Opcode+pos+4));
                }
                else{ // other modes
                    (*Disasm)->OpcodeSize=2;
                    FOpcode=(BYTE)(*(*Opcode+pos+2));
                }
            }
            break;
            
            case 1: // 1 Byte Extension to regMem
            {
                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                FOpcode=wOp&0x00FF;
                
                if(FOpcode>0x7F) // check for signed numbers
                {
                    FOpcode = 0x100-FOpcode; // -XX
                }

				++(*(*index)); // 1 byte read
                (*Disasm)->OpcodeSize=3;
                FOpcode=(BYTE)(*(*Opcode+pos+3));
            }
            break;
            
            case 2: // 2 Bytes Extension to RegMem
            {
                SwapDword((BYTE*)(*Opcode+pos),&dwOp,&dwMem);
                SwapWord((BYTE*)(*Opcode+pos+2),&wOp,&wMem);

				(*Disasm)->OpcodeSize=4;
                (*(*index))+=2; // we read 2 bytes
                FOpcode=(BYTE)(*(*Opcode+pos+4));
            }
            break;
		}
        
        switch(Bit_d)
        {
            case 0: // direction (->)
			{
                switch(Op)
                {
                    case 0x00:
                    {
                    }
                    break;

                    case 0x01:
                    {
                    }
                    break;

                    case 0x11: 
                        {
                            if(RepPrefix==1)
                            {
                                (*Disasm)->OpcodeSize++;
                            }
                        }
                        break; // MOVUPS
                    case 0x13: break; // MOVLPS
                    case 0x17: break; // MOVHPS
                    case 0x29: break; // MOVAPS
                    case 0x7E: case 0x7F: break; // MOVD/MOVQ
                    case 0xA3: case 0xAB:
                    {
                    }
                    break;

                    case 0xA4:case 0xAC:
                    {
                       (*Disasm)->OpcodeSize++;
                       (*(*index))++;
                    }
                    break;

                    case 0xA5: case 0xAD:
                    {
                    }
                    break;

                    case 0xB0: case 0xB1: case 0xB3: case 0xBB:
                    {
                      if((Op&0x0F)==0x00)
                         RM=REG8;
                    }
                    break;

                    case 0xC0:
                    { 
                        RM=REG8; 
                    } 
                    break; // XADD

                    case 0xC1: break;
                    case 0xE7: break;

                }
            }
            break;

            case 1: // direction (<-)
            {
                switch(Op)
                {                
                    case 0x10: 
                        {
                          if(RepPrefix==1)
                            (*Disasm)->OpcodeSize++;
                        }
                        break; // MOVUPS
                                        
                    case 0x2A: case 0x2C: case 0x2D:case 0x2E: case 0x2F: 
                    {
                      BYTE R=((Op&0x0F)-0x08);
                        
                      if(RepPrefix==1)
                      {
                          if(Op==0x2A || Op==0x2C || Op==0x2D)
							  (*Disasm)->OpcodeSize++;
                      }
                    }
                    break; // MIX
                    
                    case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:
                    case 0x58:case 0x59:case 0x5C:case 0x5D:case 0x5E:case 0x5F:
                    {
                        if(RepPrefix==1)
                            (*Disasm)->OpcodeSize++;
                    }
                    break; // MIX

                    case 0x70:
                    {                       
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;

                    case 0xC2:
                    {
                      if(FOpcode<8) // Instructions here
                      {
                          if(RepPrefix==1) // Rep Prefix is being used
                              (*Disasm)->OpcodeSize++;
                      }
                      else
                      {
                          if(RepPrefix==1) // Rep Prefix is being used
                              (*Disasm)->OpcodeSize++;
                      }
                       
                      (*Disasm)->OpcodeSize++;
                      (*(*index))++;
                    }
                    break;

                    case 0xC4:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;
                    
                    case 0xC5:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;
                    
                    case 0xC6:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;
                }
            }
            break;

        }

        ++(*(*index)); // add 1 byte to index
        // no need to continue!! exit the function and proeed with decoding next bytes.
		return;
    }
    
    //===================================================//
    //               NO SIB Being used!                  //
    //===================================================//
    
    if(SIB!=SIB_EX) // NO SIB extension (i.e: 0x0001 = add byte ptr [ecx], al)
    {
        reg1=((BYTE)(*(*Opcode+pos+1))&0x07); // get register (we have only one)
        reg2=(((BYTE)(*(*Opcode+pos+1))&0x38)>>3);
        
        switch(Extension) // Check what extension we have (None/Byte/Dword)
        {
            case 00: // no extention to regMem
            {
                if(reg1==REG_EBP) // cannot display EBP as memoryReg, use DWORD mem location
                {
                    SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                    SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);                    

					(*Disasm)->OpcodeSize=6;
                    (*(*index))+=5;
                    FOpcode=(BYTE)(*(*Opcode+pos+6));
                }
                else{
                    SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);

					++(*(*index)); // only 1 byte read
                    (*Disasm)->OpcodeSize=2; // total used opcodes
                    FOpcode=(BYTE)(*(*Opcode+pos+2));
                }
            }
            break;
            
            case 01: // 1 btye extention to regMem
            {
                FOpcode=(BYTE)(*(*Opcode+pos+2));
                SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);

                if(FOpcode>0x7F) // check for signed numbers
                    FOpcode = 0x100-FOpcode; // -XX
                
                if(reg1==REG_EBP && PrefixSeg==0)
                    SEG=SEG_SS;
                
                (*(*index))+=2; // x + 1 byte(s) read
                (*Disasm)->OpcodeSize=3; // total used opcodes
                FOpcode=(BYTE)(*(*Opcode+pos+3));
            }
            break;
            
            case 02: // 4 btye extention to regMem
            {
                // if ebp and there is no prefix 0x67, use SS segment
                if(reg1==REG_EBP && PrefixSeg==0)
                    SEG=SEG_SS;
                
                SwapDword((BYTE*)(*Opcode+pos+2),&dwOp,&dwMem);
                SwapWord((BYTE*)(*Opcode+pos),&wOp,&wMem);                 

				(*(*index))+=5; // x + 1 + 4 byte(s) read
                (*Disasm)->OpcodeSize=6; // total used opcodes
                FOpcode=(BYTE)(*(*Opcode+pos+6));
            }
            break;            
        }
        
        switch(Bit_d)
        {            
            case 0: // direction (->)
            {
                switch(Op) // Check for all Cases Availble            
                {
                    case 0x11: 
                        {
                            if(RepPrefix==1)
                                (*Disasm)->OpcodeSize++;
                        }
                        break; // MOVUPS
                    case 0xA4:case 0xAC:
                    {
                       (*Disasm)->OpcodeSize++;
                       (*(*index))++;
                    }
                    break;
                    
                    case 0xB0: case 0xB1:  case 0xB3: case 0xBB:
                    {
                        if((Op&0x0F)==0x00)
                            RM=REG8;
                    }
                    break;
                    case 0xC0: RM=REG8; break;
                }
            }
            break;

            case 1: // direction (<-)
            {
                switch(Op)
                {                 
					case 0x0D:
					{
						FOpcode=(BYTE)(*(*Opcode+pos+1));
					}
					break;

					case 0x10: 
                    {
						if(RepPrefix==1) // rep prefix is being used
							(*Disasm)->OpcodeSize++;
                    }
                    break; // MOVUPS
                    
                    case 0x2A: case 0x2C: case 0x2D:case 0x2E: case 0x2F: 
                    {
                       if(RepPrefix==1)
					   {
                           if(Op==0x2A || Op==0x2C || Op==0x2D)
                               (*Disasm)->OpcodeSize++;
                       }
                    }
                    break; // MIX

                    case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:
                    case 0x58:case 0x59:case 0x5C:case 0x5D:case 0x5E:case 0x5F:
                    {
                        if(RepPrefix==1)
                            (*Disasm)->OpcodeSize++;
                    }
                    break;

                    case 0x70: // PSHUFW
                    {                       
                       (*Disasm)->OpcodeSize++;
                       (*(*index))++;
                    }
                    break;

                    case 0xC2:
                    {
                       if(FOpcode<8) // Instructions here
                       {
                           if(RepPrefix==1) // Rep Prefix is being used
                               (*Disasm)->OpcodeSize++;
                       }
                       else
                       {
                           if(RepPrefix==1) // Rep Prefix is being used
                               (*Disasm)->OpcodeSize++;
                       }

                       (*Disasm)->OpcodeSize++;
                       (*(*index))++;
                    }
                    break;
                    
                    case 0xC4:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;

                    case 0xC5:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;

                    case 0xC6:
                    {
                        (*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;
                }
            }
            break;
        }

        return; // safe exit
    }
    // ===================================================//
    //                 SIB is being used!                 //
    // ===================================================//
	else if(SIB==SIB_EX) // Found SIB, lets strip the extensions
	{
		/* 
		   Example menemonic for SIB: 
		   Opcodes:   000401  
		   Menemonic: add byte ptr [eax+ecx], al
		   Binary:    0000 0000 0000 0100 0000 0001
        */
		reg1=((BYTE)(*(*Opcode+pos+2))&0x38)>>3;  // Register A
		reg2=((BYTE)(*(*Opcode+pos+2))&0x07);     // Register B
		SCALE=((BYTE)(*(*Opcode+pos+2))&0xC0)>>6; // Scale size (0,2,4,8)

		switch(Extension) // +/+00/+00000000
		{
			case 00: // No extension of bytes
			{
                SwapWord((BYTE*)(*Opcode+pos+1),&wOp,&wMem);
                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);

                if(reg1==REG_ESP && reg2!=REG_EBP)
                {
                    if(reg2==REG_ESP) SEG=SEG_SS; // IF ESP is being used, User SS Segment Overridr                    

					(*(*index))+=2; //2 byte read				
                    (*Disasm)->OpcodeSize=3; // total used opcodes
                    FOpcode=(BYTE)(*(*Opcode+pos+3));
                }
				else if(reg2!=REG_EBP) // No EBP in RegMem
				{
                    if(reg2==REG_ESP) SEG=SEG_SS; // IF ESP is being used, User SS Segment Overridr

					(*(*index))+=2; //2 byte read				
					(*Disasm)->OpcodeSize=3; // total used opcodes
                    FOpcode=(BYTE)(*(*Opcode+pos+3));
				}
				else if(reg2==REG_EBP) // Replace EBP with Dword Number
				{
					// get 4 bytes extensions for memReg addon
					// insted of Normal Registers


                    Extension=2; // OverRide Extension (?????), Check toDo.txt
					(*(*index))+=6; //6 byte read				
					(*Disasm)->OpcodeSize=7; // total used opcodes
                    FOpcode=(BYTE)(*(*Opcode+pos+6));
				}
			}
			break;

			case 01: // 1 byte extension
			{				
				FOpcode=(BYTE)(*(*Opcode+pos+3));
				if(FOpcode>0x7F) // check for signed numbers!!
				{
					FOpcode = 0x100-FOpcode; // -XX
				}
				
				if(reg2==REG_EBP || reg1==REG_ESP) // no ESP in [Mem]
                {                
					SEG=SEG_SS;
                }

				(*(*index))+=3; // x + 3 byte(s) read				
                SwapDword((BYTE*)(*Opcode+pos),&dwOp,&dwMem);

				(*Disasm)->OpcodeSize=4; // total used opcodes

				FOpcode=(BYTE)(*(*Opcode+pos+4));
			}
			break;

			case 02:// Dword extension
			{
                SwapDword((BYTE*)(*Opcode+pos+3),&dwOp,&dwMem);
                SwapWord((BYTE*)(*Opcode+pos+3),&wOp,&wMem);
				if(reg1!=REG_ESP) 
                {
                  if(reg2==REG_EBP)
                      SEG=SEG_SS;
                }

				(*(*index))+=6; // x + 3 byte(s) read	
				(*Disasm)->OpcodeSize=7; // total used opcodes

				FOpcode=(BYTE)(*(*Opcode+pos+7));
			}
			break;
		}
        switch(Bit_d)
        {            
            case 0: // direction (->)
            {
                switch(Op)// Check for all Cases Availble            
                {
                    case 0x11: 
                        {
                            if(RepPrefix==1)
                                (*Disasm)->OpcodeSize++;
                        }
                        break; // MOVUPS
                    case 0xA4:case 0xAC:
                    {
                      (*Disasm)->OpcodeSize++;
                      (*(*index))++;
                    }
                    break;

                    case 0xB0: case 0xB1: case 0xB3: case 0xBB:
                    {
                      if((Op&0x0F)==0x00)
                         RM=REG8;
                    }
                    break;
                    case 0xC0: { RM=REG8; } break; // XADD
                }
            }
            break;
            
            case 1: // direction (<-)
            {
                switch(Op) // Decode Instructions
                {
                    case 0x10: 
                        {
                            if(RepPrefix==1) // rep prefix is being used
                                (*Disasm)->OpcodeSize++;
                        }
                        break; // MOVUPS
                    
                    case 0x2A: case 0x2C: case 0x2D:case 0x2E: case 0x2F: 
                    {
                       if(RepPrefix==1)
                       {
                           if(Op==0x2A || Op==0x2C || Op==0x2D)
                               (*Disasm)->OpcodeSize++;
					   }
                    }
                    break; // MIX

                    case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:
                    case 0x58:case 0x59:case 0x5C:case 0x5D:case 0x5E:case 0x5F:
                    {
                        if(RepPrefix==1)
                            (*Disasm)->OpcodeSize++;
                    }
                    break;
                    
                    case 0x70:
                    {
                      (*Disasm)->OpcodeSize++;
                      (*(*index))++;
                    }
                    break;

                    case 0xC2:
                    {
                      if(RepPrefix==1) // Rep Prefix is being used
                          (*Disasm)->OpcodeSize++;

					  (*Disasm)->OpcodeSize++;
                      (*(*index))++;
                    }
                    break;

					case 0xC6:
					case 0xC5:
                    case 0xC4:
                    {
						(*Disasm)->OpcodeSize++;
                        (*(*index))++;
                    }
                    break;
                }
            }
            break;
        }
    }
// end
}


void SwapWord(BYTE *MemPtr,WORD *Original,WORD* Mirrored)
{
    WORD OriginalWord;
    WORD MirroredWord;
    
    _asm {
            pushad
            xor eax,eax
            mov edi,MemPtr
            mov ax,word ptr[edi]
            mov MirroredWord,ax  // 1312
            bswap eax
            shr eax,16
            mov OriginalWord,ax  // 1213 
            popad
    }
    
    *Original = OriginalWord;
    *Mirrored = MirroredWord;
    
}

void SwapDword(BYTE *MemPtr,DWORD *Original,DWORD* Mirrored)
{
    DWORD OriginalDword;
    DWORD MirroredDword;
    
    _asm {
            pushad
            mov edi,MemPtr
            mov eax,dword ptr[edi]
            mov MirroredDword,eax  // 15141312
            bswap eax
            mov OriginalDword,eax  // 12131415 
            popad
    }
    
    *Original = OriginalDword;
    *Mirrored = MirroredDword;
}
