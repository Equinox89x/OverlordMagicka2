#include "stdafx.h"
#include "SpriteFontLoader.h"

SpriteFont* SpriteFontLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
	{
		Logger::LogError(L"Failed to read the assetFile!\nPath: \'{}\'", loadInfo.assetSubPath);
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout

	//Parse the Identification bytes (B,M,F)
	const byte b{ pReader->Read<byte>() };
	const byte m{ pReader->Read<byte>() };
	const byte f{ pReader->Read<byte>() };
	//If Identification bytes doesn't match B|M|F,
	if (b != 'B' || m != 'M' || f != 'F')
	{
		//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid .fnt font");
		//return nullptr
		return nullptr;
	}

	//Parse the version (version 3 required)
	pReader->SetBufferPosition(3);
	char version{ pReader->Read<char>()};
	//If version is < 3,
	if (version != 3)
	{
		//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		//return nullptr
		return nullptr;
	}

	//Valid .fnt file >> Start Parsing!
	//use this SpriteFontDesc to store all relevant information (used to initialize a SpriteFont object)
	SpriteFontDesc fontDesc{};

	//**********
	// BLOCK 0 *
	//**********
	pReader->SetBufferPosition(4);
	//Retrieve the blockId and blockSize
	uint8_t blockId{ pReader->Read<uint8_t>() };
	fontDesc.fontSize = pReader->Read<int16_t>();
	//Retrieve the FontSize [fontDesc.fontSize]
	uint32_t blockSize{ pReader->Read<uint32_t>() };
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	pReader->MoveBufferPosition(12);
	//Retrieve the FontName [fontDesc.fontName]
	fontDesc.fontName = pReader->ReadNullString();

	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<uint8_t>();
	blockSize = pReader->Read<uint32_t>();
	//Retrieve Texture Width & Height [fontDesc.textureWidth/textureHeight]
	pReader->MoveBufferPosition(4);
	fontDesc.textureWidth = pReader->Read<uint16_t>();
	fontDesc.textureHeight = pReader->Read<uint16_t>();

	//Retrieve PageCount
	uint16_t pages{ pReader->Read<uint16_t>() };
	//> if pagecount > 1
	if (pages > 1)
	{
		//	> Log Error (Only one texture per font is allowed!)
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed");
		return nullptr;
	}
	//Advance to Block2 (Move Reader)
	pReader->MoveBufferPosition(5);

	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<uint8_t>();
	blockSize = pReader->Read<uint32_t>();
	//Retrieve the PageName (BinaryReader::ReadNullString)
	std::wstring pageName{ pReader->ReadNullString() };
	if (pageName.length() == 0)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty]");
		return nullptr;
	}
	//Construct the full path to the page texture file
	//	>> page texture should be stored next to the .fnt file, pageName contains the name of the texture file
	//	>> full texture path = asset parent_path of .fnt file (see loadInfo.assetFullPath > get parent_path) + pageName (filesystem::path::append)
	std::wstring fullPath{ loadInfo.assetFullPath.parent_path().append(pageName) };
	//	>> Load the texture (ContentManager::Load<TextureData>) & Store [fontDesc.pTexture]
	fontDesc.pTexture = ContentManager::Load<TextureData>(fullPath);
	if (fontDesc.pTexture == nullptr)
	{
		Logger::LogError(L"SpriteFontLoader::LoadContant > Texture not found");
		return nullptr;
	}




	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<uint8_t>();
	blockSize = pReader->Read<uint32_t>();
	//Retrieve Character Count (see documentation)
	int count{ static_cast<int>(blockSize / 20) };

	//Create loop for Character Count, and:
	//(loop restarts till all metrics are parsed)
	for (int i{ 0 }; i < count; i++)
	{
		//> Retrieve CharacterId (store Local) and cast to a 'wchar_t'
		wchar_t characterId{ static_cast<wchar_t>(pReader->Read<uint32_t>()) };

		//> Create instance of FontMetric (struct)
		FontMetric fontMetric{};
		//	> Set Character (CharacterId) [FontMetric::character]
		fontMetric.character = characterId;
		//	> Retrieve Xposition (store Local)
		uint16_t posx{ pReader->Read<uint16_t>() };
		//	> Retrieve Yposition (store Local)
		uint16_t posy{ pReader->Read<uint16_t>() };
		//	> Retrieve & Set Width [FontMetric::width]
		fontMetric.width = pReader->Read<uint16_t>();
		//	> Retrieve & Set Height [FontMetric::height]
		fontMetric.height = pReader->Read<uint16_t>();
		//	> Retrieve & Set OffsetX [FontMetric::offsetX]
		fontMetric.offsetX = pReader->Read<uint16_t>();
		//	> Retrieve & Set OffsetY [FontMetric::offsetY]
		fontMetric.offsetY = pReader->Read<uint16_t>();
		//	> Retrieve & Set AdvanceX [FontMetric::advanceX]
		fontMetric.advanceX = pReader->Read<uint16_t>();
		//	> Retrieve & Set Page [FontMetric::page]
		fontMetric.page = pReader->Read<unsigned char>();

		//	> Retrieve Channel (BITFIELD!!!) 
		//		> See documentation for BitField meaning [FontMetrix::channel]
		//
		auto channel{ pReader->Read<uint8_t>() };
		if (channel == 1) {
			fontMetric.channel = 2;
		}
		else if (channel == 2) {
			fontMetric.channel = 1;
		}
		else if (channel == 4) {
			fontMetric.channel = 0;
		}
		else if (channel == 8){
			fontMetric.channel = 3;
		}

		//	> Calculate Texture Coordinates using Xposition, Yposition, fontDesc.TextureWidth & fontDesc.TextureHeight [FontMetric::texCoord]
		XMFLOAT2 texCoord{ fontDesc.pTexture->GetDimension() };
		fontMetric.texCoord = XMFLOAT2{ posx / texCoord.x, posy / texCoord.y };

		//> Insert new FontMetric to the metrics [font.metrics] map
		//	> key = (wchar_t) charId
		//	> value = new FontMetric
		fontDesc.metrics.emplace(characterId, fontMetric);
	}

	//Done!
	delete pReader;
	return new SpriteFont(fontDesc);
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
