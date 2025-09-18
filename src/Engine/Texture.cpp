#include "Engine/Texture.h"

#include "directxtk12/DDSTextureLoader.h"

// コンストラクタ
Texture::Texture() : m_pTex(nullptr), m_pPool(nullptr), m_index(0) {}

// デストラクタ
Texture::~Texture() { Term(); }

// 初期化
bool Texture::Init() {}