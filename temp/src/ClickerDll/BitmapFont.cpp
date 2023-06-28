#include "stdafx.h"
#include ".\BitmapFont.h"
#include "RawImage.h"



/**  
 * \brief 
 */
BitmapFont::BitmapFont()
{
	CRawImage font;
	font.LoadBitmapFile(L"C:\\Users\\KTemelkov\\Projects\\Personal\\LordOfMU3\\src\\ClickerDll\\font.bmp");


	glEnable(GL_TEXTURE_2D);
	glGenTextures(1,&textureId);
	glBindTexture(GL_TEXTURE_2D,textureId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font.width(), font.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, font.PixelBuffer());
		
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	float cx;
	float cy;

	base = glGenLists(256);
	
	for(int i = 0; i < 256; i++)
	{
		cx = float(i%16)/16.0f;
		cy = float(i/16)/16.0f;
	
		glNewList(base+i,GL_COMPILE);
		glBegin(GL_QUADS);
			glTexCoord2f(cx,cy+0.0625f);
			glVertex2i(0,16);
			glTexCoord2f(cx+0.0625f,cy+0.0625f);
			glVertex2i(16,16);
			glTexCoord2f(cx+0.0625f,cy);
			glVertex2i(16,0);
			glTexCoord2f(cx,cy);
			glVertex2i(0,0);
		glEnd();
		glTranslated(10,0,0);
		glEndList();
	}

	glDisable(GL_TEXTURE_2D);

//	base = 1000;

//	HDC hdc = CreateDC(L"DISPLAY", 0, 0, 0);
//	wglUseFontBitmaps(hdc, 0, 255, base); 

//	DeleteDC(hdc);
}


BitmapFont::~BitmapFont()
{
	glDeleteLists(base,256);
}

void BitmapFont::drawString(int x, int y, const std::string &text)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textureId);
	glTranslatef(static_cast<GLfloat>(x),static_cast<GLfloat>(y),0.0f);
	glListBase(base);
	glCallLists(static_cast<GLsizei>(text.length()),GL_UNSIGNED_BYTE,text.c_str());
	glDisable(GL_TEXTURE_2D);
/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(static_cast<GLfloat>(x),static_cast<GLfloat>(y),0.0f);

	glListBase (base); 
	glCallLists(static_cast<GLsizei>(text.length()),GL_UNSIGNED_BYTE,text.c_str());
*/
}

const std::string BitmapFont::getFontName() const
{
	return "Bitmap";
}

std::size_t BitmapFont::getSize() const
{
	return 10;
}

ui::util::Dimension BitmapFont::getStringBoundingBox(const std::string &text) const
{
	int width = static_cast<int>(text.length() * getSize());
	return ui::util::Dimension(width,static_cast<int>(getSize()));
}
