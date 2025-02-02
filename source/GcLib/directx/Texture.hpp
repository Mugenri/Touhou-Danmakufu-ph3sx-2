#pragma once

#include "../pch.h"

#include "DxConstant.hpp"
#include "DirectGraphics.hpp"

namespace directx {
	class TextureData;
	class Texture;
	class TextureManager;
	class TextureInfoPanel;

	//****************************************************************************
	//Texture
	//****************************************************************************
	class TextureData {
		friend Texture;
		friend TextureManager;
		friend TextureInfoPanel;
	public:
		enum class Type : uint8_t {
			TYPE_TEXTURE,
			TYPE_RENDER_TARGET,
		};
	protected:
		TextureManager* manager_;
		volatile bool bReady_;
		
		Type type_;
		std::wstring name_;
		D3DXIMAGE_INFO infoImage_;
		size_t resourceSize_;

		bool useMipMap_;
		bool useNonPowerOfTwo_;

		IDirect3DTexture9* pTexture_;
		IDirect3DSurface9* lpRenderSurface_;
		IDirect3DSurface9* lpRenderZ_;
	public:
		TextureData();
		virtual ~TextureData();

		std::wstring& GetName() { return name_; }
		D3DXIMAGE_INFO* GetImageInfo() { return &infoImage_; }

		IDirect3DTexture9* GetD3DTexture() { return pTexture_; }
		IDirect3DSurface9* GetD3DSurface() { return lpRenderSurface_; }
		IDirect3DSurface9* GetD3DZBuffer() { return lpRenderZ_; }

		size_t GetResourceSize() { return resourceSize_; }
		void CalculateResourceSize();
	};

	class Texture : public gstd::FileManager::LoadObject {
		friend TextureData;
		friend TextureManager;
		friend TextureInfoPanel;
	protected:
		shared_ptr<TextureData> data_;
	public:
		Texture();
		Texture(Texture* texture);
		virtual ~Texture();

		void Release();

		std::wstring GetName();
		bool CreateFromData(const std::wstring& name);
		bool CreateFromData(shared_ptr<TextureData> data);
		bool CreateFromFile(const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo);
		bool CreateRenderTarget(const std::wstring& name, size_t width = 0U, size_t height = 0U);
		bool CreateFromFileInLoadThread(const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo, bool bLoadImageInfo = false);

		auto GetTextureData() { return data_; }
		void SetTexture(IDirect3DTexture9 *pTexture);

		IDirect3DTexture9* GetD3DTexture();
		IDirect3DSurface9* GetD3DSurface();
		IDirect3DSurface9* GetD3DZBuffer();

		TextureData::Type GetType();

		UINT GetWidth();
		UINT GetHeight();
		bool IsLoad() { return data_ != nullptr && data_->bReady_; }

		static size_t GetFormatBPP(D3DFORMAT format);
	};

	//****************************************************************************
	//TextureManager
	//****************************************************************************
	class TextureManager : public DirectGraphicsListener, public gstd::FileManager::LoadThreadListener {
		friend Texture;
		friend TextureData;
		friend TextureInfoPanel;
		static TextureManager* thisBase_;
	public:
		static const std::wstring TARGET_TRANSITION;
	protected:
		gstd::CriticalSection lock_;

		std::map<std::wstring, shared_ptr<Texture>> mapTexture_;
		std::map<std::wstring, shared_ptr<TextureData>> mapTextureData_;
		std::list<std::pair<std::map<std::wstring, shared_ptr<TextureData>>::iterator, IDirect3DSurface9*>> listRefreshSurface_;
		shared_ptr<TextureInfoPanel> panelInfo_;

		void _ReleaseTextureData(const std::wstring& name);
		void _ReleaseTextureData(std::map<std::wstring, shared_ptr<TextureData>>::iterator itr);

		void __CreateFromFile(shared_ptr<TextureData>& dst, const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo);
		bool _CreateFromFile(shared_ptr<TextureData>& dst, const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo);
		bool _CreateRenderTarget(shared_ptr<TextureData>& dst, const std::wstring& name, 
			size_t width = 0U, size_t height = 0U);
	public:
		TextureManager();
		virtual ~TextureManager();

		static TextureManager* GetBase() { return thisBase_; }
		
		virtual bool Initialize();
		gstd::CriticalSection& GetLock() { return lock_; }

		virtual void Clear();
		virtual void Add(const std::wstring& name, shared_ptr<Texture> texture);
		virtual void Release(const std::wstring& name);
		virtual void Release(std::map<std::wstring, shared_ptr<Texture>>::iterator itr);
		virtual bool IsDataExists(const std::wstring& name);
		virtual std::map<std::wstring, shared_ptr<TextureData>>::iterator IsDataExistsItr(const std::wstring& name, bool* res = nullptr);

		virtual void ReleaseDxResource();
		virtual void RestoreDxResource();

		shared_ptr<TextureData> GetTextureData(const std::wstring& name);
		shared_ptr<Texture> GetTexture(const std::wstring& name);
		
		shared_ptr<Texture> CreateFromFile(const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo);
		shared_ptr<Texture> CreateRenderTarget(const std::wstring& name, size_t width = 0U, size_t height = 0U);
		
		shared_ptr<Texture> CreateFromFileInLoadThread(const std::wstring& path, bool genMipmap, bool flgNonPowerOfTwo, bool bLoadImageInfo = false);
		virtual void CallFromLoadThread(shared_ptr<gstd::FileManager::LoadThreadEvent> event);

		void SetInfoPanel(shared_ptr<TextureInfoPanel> panel) { panelInfo_ = panel; }
	};

	//****************************************************************************
	//TextureInfoPanel
	//****************************************************************************
	class TextureInfoPanel : public gstd::ILoggerPanel {
		struct TextureDisplay {
			enum Column {
				Address,
				Name, FullPath,
				Uses, Size,
				_NoSort,
			};

			uintptr_t address;
			std::string strAddress;
			std::string fileName;
			std::string fullPath;
			int countRef;
			uint32_t wd;
			uint32_t ht;
			uint32_t size;

			static const ImGuiTableSortSpecs* imguiSortSpecs;
			static bool IMGUI_CDECL Compare(const TextureDisplay& a, const TextureDisplay& b);
		};
	protected:
		std::vector<TextureDisplay> listDisplay_;
		uint32_t videoMem_;
	public:
		TextureInfoPanel();

		virtual void Initialize(const std::string& name);

		virtual void Update();
		virtual void ProcessGui();
	};
}