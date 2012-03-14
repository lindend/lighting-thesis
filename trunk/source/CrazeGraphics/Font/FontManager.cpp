#include "CrazeGraphicsPCH.h"
#include "FontManager.h"

#include <utility>
#include <sstream>

#include "windows.h"
#include "Shlobj.h"

#include "Resource/ResourceManager.h"
#include "Resource/FileDataLoader.h"

#include "../Device.h"
#include "../Graphics.h"
#include "../Material.h"
#include "../Texture/Texture.h"
#include "../Buffer/Buffer.h"
#include "FontVertex.h"

using namespace Craze;
using namespace Graphics2;

namespace Craze
{
	namespace Graphics2
	{
		FontManager gFontMgr;
	}
}

std::string getFontPath()
{
	wchar_t* pOut;
	SHGetKnownFolderPath(FOLDERID_Fonts, 0, 0, &pOut);
	std::string res = WToStr(pOut);
	CoTaskMemFree(pOut);
	return res;
}

bool Craze::Graphics2::InitFont()
{
	int error = FT_Init_FreeType(&gFontMgr.m_library);

	if (error)
	{
		return false;
	}

	const char *paths[] = { "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
							"SOFTWARE\\Microsoft\\WindowsNT\\CurrentVersion\\Fonts",
							"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts" };
	for (int i = 0; i < 3; ++i)
	{
		HKEY fontsKey;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, paths[i], 0, KEY_READ, &fontsKey) == ERROR_SUCCESS)
		{
			std::string fontPath = getFontPath();

			const int BufSize = 32000;
			char value[BufSize];
			DWORD valueSize = BufSize;
			char data[BufSize];
			DWORD dataSize = BufSize;
			DWORD type;
			int i = 0;
			while (RegEnumValueA(fontsKey, i, value, &valueSize, 0, &type, (LPBYTE)data, &dataSize) != ERROR_NO_MORE_ITEMS)
			{
				valueSize = BufSize;
				dataSize = BufSize;
				++i;

				std::string v = value;
				int pos = v.find("(TrueType)");
				if (pos == std::string::npos || type != REG_SZ)
				{
					continue;
				}

				v = v.substr(0, pos - 1);
				gFontMgr.m_fontFiles[v] = fontPath + "\\" + data;
			}

			break;
		}
	}

	return true;

}

void StaticString::render(int xpos, int ypos)
{
	Matrix4 world = Matrix4::CreateTranslation((float)xpos, (float)ypos, 0.f);
	Material mat;
	mat.m_decal = m_face->getTexture();

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	gpDevice->SetObjectProperties(world, mat);
	gpDevice->drawBuffer(m_verts);

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

std::vector<std::vector<std::string>> SplitToLinesOfWords(std::string text)
{
	//Inner vector contains all words of a line, outer contains all lines.
	std::vector<std::vector<std::string>> words;
	words.push_back(std::vector<std::string>());

	std::string lastWord = "";

	for (unsigned int i = 0; i < text.length(); ++i)
	{
		char l = text.at(i);

		if (l == ' ')
		{
			words.back().push_back(lastWord);
			lastWord.clear();
		} else if(l == '\n')
		{
			words.back().push_back(lastWord);
			words.push_back(std::vector<std::string>());
			lastWord.clear();
		} else
		{
			lastWord += l;
		}
	}
	words.back().push_back(lastWord);

	return words;
}

void Face::buildString(const std::string &text, const Vec3 &color, unsigned int maxWidth, FontVertex *&outVerts, int &outNumVerts)
{
	unsigned int spaceWidth;
	auto swit = m_coords.find(' ');
	spaceWidth = swit->second.advance;

	int penX = 0;
	int penY = 0;
	int idx = 0;

	outVerts = (FontVertex*)gMemory.FrameAlloc(sizeof(FontVertex) * text.length());

	auto words = SplitToLinesOfWords(text);

	for (auto i = words.begin(); i != words.end(); ++i)
	{
		for (auto j = i->begin(); j != i->end(); ++j)
		{
			//Check if a new linebreak should be performed due to line overflow
			if (maxWidth && getStringWidth(*j) + penX > maxWidth)
			{
				penX = 0;
				penY += m_face->height;
			} 

			for (unsigned int c = 0; c < j->length(); ++c)
			{
				char c1 = j->at(c);
				auto dataIt = m_coords.find(c1);
				
				if (dataIt != m_coords.end())
				{
					GlyphData data = dataIt->second;
					FontVertex v;
					outVerts[idx].position = Vector2((float)(penX + data.xOffset), (float)(penY + data.yOffset));
					outVerts[idx].size = data.size;
					outVerts[idx].startUV = data.start;
					outVerts[idx].endUV = data.end;
					++idx;

					char c2 = (j->length() - 1) - c > 0 ? j->at(c + 1) : ' '; 
					penX += getAdvance(c1, c2);
				}
			}

			penX += spaceWidth;
		}

		penX = 0;
		penY += m_face->height;
	}
	outNumVerts = idx;
}

StaticString Face::cacheStaticString(const std::string& text, const Vec3& color, unsigned int maxWidth)
{
	FontVertex* vertices;
	int numVerts;
	buildString(text, color, maxWidth, vertices, numVerts);

	StaticString str;
	str.m_verts = GeometryBuffer::Create(gpDevice, GeometryBuffer::VERTEX, vertices, sizeof(FontVertex), numVerts, false, "Static string");
	str.m_face = this;

	return str;
}

bool Face::renderString(const std::string &text, int xpos, int ypos, const Vec3 &color, unsigned int maxWidth)
{
	FontVertex* vertices;
	int numVerts;
	buildString(text, color, maxWidth, vertices, numVerts);

	m_vertBuffer->WriteData(vertices, numVerts);

	Matrix4 world = Matrix4::CreateTranslation((float)xpos, (float)ypos + m_size, 0.f);
	Material mat;
	mat.m_decal = getTexture();

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	gpDevice->SetObjectProperties(world, mat);
	gpDevice->drawBuffer(m_vertBuffer);

	gpDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

unsigned int Face::getStringWidth(std::string text)
{
	unsigned int width = 0;
	for (unsigned int i = 0; i < text.length(); ++i)
	{
		char c1 = text.at(i);

		if (i != text.length() - 1)
		{
			width += getAdvance(c1, text.at(i + 1));
		} else
		{
			auto it = m_coords.find(c1);
			if (it != m_coords.end())
			{
				width += (int)(it->second.size.x + 0.5f);
			}
		}
	}

	return width;
}

unsigned int Face::getAdvance(char c1, char c2)
{
	auto it = m_coords.find(c1);
	if (it != m_coords.end())
	{
		return it->second.advance;
	}

	return 0;
}

//Create a list of all glyps, with bitmaps, that is ready to be packed
struct GlyphBitmap
{
    int glyphIndex;
    char* pGlyphBitmap;

    unsigned int width;
    unsigned int height;

    int xOffset;
    int yOffset;

    int advance;
};

std::vector<GlyphBitmap> renderGlyphs(FT_Face f, std::string chars)
{
    std::vector<GlyphBitmap> res;
    res.reserve(chars.length());

    FT_GlyphSlot slot = f->glyph;

    for (unsigned int i = 0; i < chars.length(); ++i)
    {
		char c = chars.at(i);

        GlyphBitmap bmp;
        bmp.glyphIndex = FT_Get_Char_Index(f, c);

        if (FT_Load_Glyph(f, bmp.glyphIndex, FT_LOAD_RENDER) != 0)
		{
			//error!
		}

        bmp.xOffset = slot->bitmap_left;
        bmp.yOffset = -slot->bitmap_top;
        bmp.width = slot->bitmap.width;
        bmp.height = slot->bitmap.rows;

        bmp.pGlyphBitmap = new char[bmp.width * bmp.height];

        for (unsigned int y = 0; y < bmp.height; ++y)
        {
            for (unsigned int x = 0; x < bmp.width; ++x)
            {
                bmp.pGlyphBitmap[x + y * bmp.width] = slot->bitmap.buffer[x + y * slot->bitmap.pitch];
            }
        }
		
        bmp.advance = slot->advance.x >> 6;

        res.push_back(bmp);
    }

	return res;
}

void freeGlyphBitmaps(std::vector<GlyphBitmap>& bmps)
{
	for(auto i = bmps.begin(); i != bmps.end(); ++i)
	{
		delete [] i->pGlyphBitmap;
	}
}

struct GlyphPos
{
	unsigned int x;
	unsigned int y;
};

struct PackedGlyphs
{
    std::vector<GlyphPos> positioning;

    unsigned int textureWidth;
    unsigned int textureHeight;
};

PackedGlyphs packGlyphs(std::vector<GlyphBitmap>& glyphs, unsigned int maxTextureWidth = 64)
{
    //Not using a sophisticated packing algorithm... yet. Hopefully this one turns out to be
    //sufficiently good :]

    PackedGlyphs pg;
    pg.positioning.reserve(glyphs.size());

    int currentX = 0;
    int currentY = 0;
    unsigned int lineMaxSize = 0;

    for (auto i = glyphs.begin(); i != glyphs.end(); ++i)
    {
        if (currentX + i->width + 1 >= maxTextureWidth)
        {
            currentX = 0;
            currentY += lineMaxSize + 1;
            lineMaxSize = i->height;
        }

        GlyphPos data;
        data.x = currentX; 
        data.y = currentY;

        currentX += i->width + 1;
        lineMaxSize = Max(i->height, lineMaxSize);

        pg.positioning.push_back(data);
    }

    pg.textureWidth = maxTextureWidth;
    pg.textureHeight = currentY + lineMaxSize + 1;

    return pg;
}

void copyGlyphBitmap(char* dest, unsigned int destPitch, GlyphBitmap src, GlyphPos dstPos)
{
	for (unsigned int y = 0; y < src.height; ++y)
	{
		for (unsigned int x = 0; x < src.width; ++x)
		{
			dest[dstPos.x + x + (dstPos.y + y) * destPitch] = src.pGlyphBitmap[x + y * src.width];
		}
	}
}

Texture* createFontTexture(Device* pDevice, PackedGlyphs& pg, std::vector<GlyphBitmap>& bmps)
{
    char* textureData = new char[pg.textureHeight * pg.textureWidth];
	ZeroMemory(textureData, pg.textureHeight * pg.textureWidth);

    for (unsigned int i = 0; i < bmps.size(); ++i)
    {
		copyGlyphBitmap(textureData, pg.textureWidth, bmps[i], pg.positioning[i]);
    }

	auto ret = Texture::CreateFromData(pDevice, pg.textureWidth, pg.textureHeight, TEXTURE_FORMAT_8BIT_UNORM, textureData, "font texture");

	delete [] textureData;

	return ret;
}

std::vector<GlyphData> buildGlyphData(PackedGlyphs& pg, std::vector<GlyphBitmap>& bmps)
{
	std::vector<GlyphData> gd;
	gd.reserve(bmps.size());

	for (unsigned int i = 0; i < bmps.size(); ++i)
	{
		GlyphData data;
		data.advance = bmps[i].advance;
		data.xOffset = bmps[i].xOffset;
		data.yOffset = bmps[i].yOffset;

		data.size = Vector2((float)bmps[i].width, (float)bmps[i].height);

		data.start.x = (float)pg.positioning[i].x / (float)pg.textureWidth;
		data.start.y = (float)pg.positioning[i].y / (float)pg.textureHeight;
		data.end.x = data.start.x + (float)bmps[i].width / (float)pg.textureWidth;
		data.end.y = data.start.y + (float)bmps[i].height / (float)pg.textureHeight;

		gd.push_back(data);
	}

	return gd;
}

void FontManager::shutdown()
{
	for (auto i = m_faces.begin(); i != m_faces.end(); ++i)
	{
		FT_Done_Face(i->second);
	}

	m_resources.clear();

	FT_Done_FreeType(m_library);
}

bool FontManager::addFontFile(const std::string &fontFile)
{
	std::shared_ptr<const DefaultResource> res = std::dynamic_pointer_cast<const DefaultResource>(gResMgr.loadResourceBlocking(gFileDataLoader.addFile(fontFile)));

	if (!res)
	{
		return false;
	}
	if (res->getStatus() != Resource::FINISHED)
	{
		return false;
	}

	m_resources.push_back(res);


	FT_Face face;
	int error = FT_New_Memory_Face(m_library, (const FT_Byte*)res->getData(), res->getSize(), 0, &face);

	if (error)
	{
	    m_lastError = "Unable to load font from file";
		return false;
	}

	std::string familyName = face->family_name;

	m_faces.insert(make_pair(familyName, face));

	int numFaces = face->num_faces;
	for (int i = 1; i < numFaces; ++i)
	{
		error = FT_New_Memory_Face(m_library, (const FT_Byte*)res->getData(), res->getSize(), i, &face);

		if (error)
		{
			continue;
		}

		std::string familyName = face->family_name;

		m_faces.insert(make_pair(familyName, face));
	}

	return true;
}

Face* FontManager::createFace(std::string familyName, std::string styleName, unsigned int size)
{
	m_lastError = 0;

	auto r = m_faces.equal_range(familyName);

	//If no such font is loaded...
	if (r.first == r.second)
	{
		auto it = m_fontFiles.find(familyName);
		if (it != m_fontFiles.end())
		{
			addFontFile(it->second);
			r = m_faces.equal_range(familyName);
		}
	}

	FT_Face f = nullptr;

	for (auto i = r.first; i != r.second; ++i)
	{
		if (i->second->style_name == styleName)
		{
			f = i->second;
			break;
		}
	}

	if (!f && r.first != r.second)
	{
		//Just default to the first of what we found
		f = r.first->second;
	} else if(!f)
	{
		m_lastError = "Unable to find specified font face";
		return nullptr;
	}

	int error = 0;
	error = FT_Set_Pixel_Sizes(f, 0, size);

	if (error)
	{
		m_lastError = "Specified font size not supported by font";
		return nullptr;
	}

	Face* pFace = CrNew Face();
	pFace->m_size = size;
	pFace->m_face = f;

	std::string str = "ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖabcdefghijklmnopqrstuvwxyzåäö0123456789 ,.-!?$\"'";
	auto bmps = renderGlyphs(f, str);
	auto packInfo = packGlyphs(bmps);
	auto glyphData = buildGlyphData(packInfo, bmps);
	auto pTexture = createFontTexture(gpDevice, packInfo, bmps);
	freeGlyphBitmaps(bmps);

	pFace->m_texture = pTexture;
	pFace->m_vertBuffer = GeometryBuffer::Create(gpDevice, GeometryBuffer::VERTEX, nullptr, sizeof(FontVertex), 32000, true, "Face vertex buffer");

	for (unsigned int i = 0; i < str.length(); ++i)
	{
		pFace->m_coords.insert(std::make_pair(str.at(i), glyphData[i]));

	}

	return pFace;
}
