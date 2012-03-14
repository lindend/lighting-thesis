#pragma once

#include <map>
#include <string>
#include <vector>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Vector2.h"

/*
Current parts of the font system:
Font manager - loads in new font files, and 
Face - should be able to render text and create cached StaticStrings
*/
namespace Craze
{
	class Resource;

	namespace Graphics2
	{
		class Device;
		class Texture;
		class GeometryBuffer;
		struct FontVertex;

		struct GlyphData
		{
			Vector2 start;
			Vector2 end;
			Vector2 size;

			unsigned int advance;

			int xOffset;
			int yOffset;
		};

		class StaticString
		{
			CRAZE_ALLOC();
			friend class Face;
		public:
			void render(int xpos, int ypos);

			StaticString() : m_face(0) {}
			StaticString(const StaticString& o) { *this = o; }
			StaticString& operator=(const StaticString& o) { m_verts = o.m_verts; m_face = o.m_face; return *this; }

		protected:
			std::shared_ptr<GeometryBuffer> m_verts;

			Face *m_face;
		};

		class Face
		{
			CRAZE_ALLOC();
			friend class FontManager;
		public:
			~Face() { m_coords.clear(); }
			
			StaticString cacheStaticString(const std::string& text, const Vec3& color, unsigned int maxWidth = 0);
			bool renderString(const std::string &text, int xpos, int ypos, const Vec3 &color, unsigned int maxWidth = 0);

			unsigned int getStringWidth(std::string text);
			unsigned int getAdvance(char c1, char c2);
			unsigned int getSize() const { return m_size; }

			Texture* getTexture() const { return m_texture; }

		private:
			Face() {}

			void buildString(const std::string &text, const Vec3 &color, unsigned int maxWidth, FontVertex*& outVerts, int &outNumVerts);

			std::map<char, GlyphData> m_coords;
			std::shared_ptr<GeometryBuffer> m_vertBuffer;

			Texture* m_texture;
			FT_Face m_face;
			unsigned int m_size;

			Face(const Face& o);
			Face& operator=(const Face& o);
		};

		bool InitFont();

		class FontManager
		{
			friend bool InitFont();

		public:
			FontManager() : m_lastError(0) {}
			void shutdown();

			bool addFontFile(const std::string &fontFile);

			std::vector<std::string> getAvailableFonts() const;

			Face* createFace(std::string familyName, std::string styleName, unsigned int size);

		private:
			FontManager(const FontManager&);
			FontManager& operator=(const FontManager&);

			FT_Library m_library;
			std::multimap<std::string, FT_Face> m_faces;
			std::map<std::string, std::string> m_fontFiles;
			std::vector<std::shared_ptr<const Resource>> m_resources;

			const char* m_lastError;
		};

		extern FontManager gFontMgr;
	}
}
