//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    
    //Texture/Spirit creation 
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
    DirectX::SimpleMath::Vector2 m_screenPos;
    DirectX::SimpleMath::Vector2 m_origin;

    //std::unique_ptr<DirectX::CommonStates> m_states;

    RECT m_fullscreenRect;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_background;

    //Drawing a triangle

    using VertexType = DirectX::VertexPositionColor;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::BasicEffect> m_effect;
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

    //3D model

    DirectX::SimpleMath::Matrix m_world;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::GeometricPrimitive> m_shape;

    //Grid 
    //Anti-aliass lines
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;
    
    //Multisampling
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_offscreenRenderTarget;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_offscreenRenderTargetSRV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilSRV;

    //Modeling
	std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;
	std::unique_ptr<DirectX::Model> m_model;
    //Mouse and keyboard
    std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

    std::unique_ptr<DirectX::GeometricPrimitive> m_room;

	float m_pitch;
	float m_yaw;

	DirectX::SimpleMath::Vector3 m_cameraPos;

	DirectX::SimpleMath::Color m_roomColor;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_roomTex;

	DirectX::Keyboard::KeyboardStateTracker m_keys;
	DirectX::Mouse::ButtonStateTracker m_mouseButtons;

};
