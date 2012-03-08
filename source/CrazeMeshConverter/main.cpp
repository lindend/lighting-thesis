#include <sstream>
#include <iostream>
#include <set>

#include "boost/filesystem.hpp"
#include "IL/il.h"
#include "IL/ilu.h"

#include "StrUtil.hpp"
#include "Assets/CrazeModel.h"
#include "CrazeGraphics/Model.h"
#include "Util/CrazeHash.h"



using namespace std;
using namespace Craze;
using namespace Craze::Graphics2;

bool CopyTexture(const std::string& from, const std::string& to)
{
	ILuint texId = ilGenImage();
	ilBindImage(texId);

	if (!ilLoadImage(StrToW(from).c_str()))
	{
		ILenum errnum = ilGetError();
		std::string error = WToStr(iluErrorString(ilGetError()));
		LOG_ERROR(error);
		ilDeleteImage(texId);
		return false;
	}

	ilSetInteger(IL_JPG_QUALITY, 100);

	if (!ilSaveImage(StrToW(to + ".jpg").c_str()))
	{
		std::string error = WToStr(iluErrorString(ilGetError()));
		LOG_ERROR(error);
		ilDeleteImage(texId);
		return false;
	}

	ilDeleteImage(texId);

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		//cmc "D:\Daniel\Imba file.obj" "D:\Test\" "Textures"
		cout << "Incorrect usage, usage: cmc <input file> <output dir> <relative texture output dir>" << endl;
		return 1;
	}

	std::string file = argv[1];
	if (file == "")
	{
		cout << "No input file selected" << endl;
		return 1;
	}

	auto meshes = loadModelAssimp(file);

	if (meshes.size() == 0)
	{
		cout << "No meshes in selected file" << endl;
		return 1;
	}

	ilInit();
	iluInit();

	std::string name = file.substr(0, file.find_last_of('.'));
	name = name.substr(file.find_last_of("/\\") + 1, std::string::npos);

	std::set<u64> savedTextures;

	unsigned int texId = 0;

	boost::filesystem::path outPath = argv[2];
	boost::filesystem::path outTexPath = outPath / argv[3];

	boost::filesystem::create_directories(outPath);
	boost::filesystem::create_directories(outTexPath);
	
	for (unsigned int i = 0; i < meshes.size(); ++i)
	{
		//Change the texture path to become relative to the model file
		std::string srcTexPath = meshes[i]->m_Material.decalFileName;
		srcTexPath = file.substr(0, file.find_last_of("/\\")) + "\\" + srcTexPath;

		u64 texId = Craze::hash64(meshes[i]->m_Material.decalFileName.c_str());

		if (savedTextures.find(texId) == savedTextures.end())
		{
			std::stringstream texName;
			//name << "_" << texId++ << ".jpg";
			texName << texId;
			CopyTexture(srcTexPath, (outTexPath / texName.str()).string());
		}

		meshes[i]->m_Material.decalId = texId;

	}

	Craze::Graphics2::saveModel((outPath / (name + ".crm")).string(), meshes);

	return 0;
}


