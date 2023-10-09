//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

//Multisampling
namespace
{
   const XMVECTORF32 START_POSITION = { 0.f, 0.25f, -3.f, 0.f };
    //The Starting position of the Camera
   const XMVECTORF32 ROOM_BOUNDS = { 80.f, 60.f, 120.f, 0.f };
   constexpr float ROTATION_GAIN = 0.004f;
   constexpr float MOVEMENT_GAIN = 0.07f;

	constexpr UINT MSAA_COUNT = 4;
    constexpr UINT MSAA_QUALITY = 0;
}

Game::Game() noexcept(false):
	m_pitch(0),
    m_yaw(0),
    m_cameraPos(START_POSITION),
    m_roomColor(Colors::White)
{
	//m_deviceResources = std::make_unique<DX::DeviceResources>(); No Multisampling
	//Multisampling

	m_deviceResources = std::make_unique<DX::DeviceResources>(
		DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
	// TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
	//   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
	//   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
	m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
    m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	static_cast<float>(timer.GetElapsedSeconds());

    static_cast<float>(timer.GetTotalSeconds());

	const auto mouse = m_mouse->GetState();

   
    
    m_mouseButtons.Update(mouse);
	if (mouse.positionMode == Mouse::MODE_RELATIVE)
	{
		const Vector3 delta = Vector3(static_cast<float>(mouse.x), static_cast<float>(mouse.y), 0.f)
	                    * ROTATION_GAIN;

	    m_pitch -= delta.y;
	    m_yaw -= delta.x;
	}

	m_mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);


    //Grid
    m_world = Matrix::CreateRotationY(cosf(static_cast<float>(timer.GetTotalSeconds())));
        //Comment this to have a swival 

	m_world = Matrix::CreateRotationZ(cosf(static_cast<float>(timer.GetTotalSeconds()) * 2.f));

    //Moves all the objects to the postion  
	//m_world = Matrix::CreateTranslation(Vector3(1.0f, 1.0f, 1.0f));


    Matrix world = Matrix::CreateTranslation( 2.f, 1.f, 3.f);
	m_effect->SetWorld(world);

	    // limit pitch to straight up or straight down
	constexpr float limit = XM_PIDIV2 - 0.01f;
	m_pitch = std::max(-limit, m_pitch);
	m_pitch = std::min(+limit, m_pitch);

	// keep longitude in sane range by wrapping
	if (m_yaw > XM_PI)
	{
	    m_yaw -= XM_2PI;
	}
	else if (m_yaw < -XM_PI)
	{
	    m_yaw += XM_2PI;
	}

	float y = sinf(m_pitch);
	float r = cosf(m_pitch);
	float z = r * cosf(m_yaw);
	float x = r * sinf(m_yaw);

	XMVECTOR lookAt = m_cameraPos + Vector3(x, y, z);

	m_view = XMMatrixLookAtRH(m_cameraPos, lookAt, Vector3::Up);


	



	const auto kb = m_keyboard->GetState();
	m_keys.Update(kb);
	if ( kb.Escape )
	{
	    ExitGame();
	}

	if (kb.Home)
	{
	    m_cameraPos = START_POSITION.v;
	    m_pitch = m_yaw = 0;
	}

	Vector3 move = Vector3::Zero;

	if (kb.Up)
	    move.y += 1.f;

	if (kb.Down)
	    move.y -= 1.f;

	if (kb.A)
	    move.x += 1.f;

	if (kb.D)
	    move.x -= 1.f;

	if (kb.W)
	    move.z += 1.f;

	if (kb.S)
	    move.z -= 1.f;

	Quaternion q = Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.f);

	move = Vector3::Transform(move, q);

	move *= MOVEMENT_GAIN;

	m_cameraPos += move;

	Vector3 halfBound = (Vector3(ROOM_BOUNDS.v) / Vector3(2.f) ) - Vector3(0.1f, 0.1f, 0.1f);

	m_cameraPos = Vector3::Min(m_cameraPos, halfBound);
	m_cameraPos = Vector3::Max(m_cameraPos, -halfBound);



    //Changes the Color of the Texture
	if (m_keys.pressed.Tab || m_mouseButtons.rightButton == Mouse::ButtonStateTracker::PRESSED)
	{
    if (m_roomColor == Colors::Red.v)
        m_roomColor = Colors::Green;
    else if (m_roomColor == Colors::Green.v)
        m_roomColor = Colors::Blue;
	else if (m_roomColor == Colors::Blue.v)
        m_roomColor = Colors::White;
	else
        m_roomColor = Colors::Red;
	}


	// limit pitch to straight up or straight down

}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    //Anti-aliased lines
    context->RSSetState(m_raster.Get());

    m_effect->Apply(context);

    context->IASetInputLayout(m_inputLayout.Get());

     // Draw procedurally generated dynamic grid
    const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
    const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };







	//Creates new background
    m_deviceResources->PIXBeginEvent(L"Draw Background");
    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_background.Get(), m_fullscreenRect);
    m_spriteBatch->End();
    m_deviceResources->PIXEndEvent();

   

	

	//3D object
	   /* m_deviceResources->PIXBeginEvent(L"Draw Sphere");
	    XMMATRIX local = m_world * Matrix::CreateTranslation(0.f, 1.5f, 0.f);
		m_shape->Draw(local, m_view, m_proj, Colors::White, m_texture.Get());
		m_deviceResources->PIXEndEvent();*/

	//3D object for loop test
    for (int k=0; k < 5; k++){
	    for (int j=0; j < 5; j++)
	    {
		    for (int i=0; i < 5; i++){
			    m_deviceResources->PIXBeginEvent(L"Draw Sphere");
			    XMMATRIX local = m_world * Matrix::CreateTranslation(k+(-2.f), i+(-2.f), j+(0.f));
				m_shape->Draw(local, m_view, m_proj, Colors::White, m_texture.Get());
				m_deviceResources->PIXEndEvent();
			}
	    }
    }

	//Grid
   

	//Cup Model
	m_deviceResources->PIXBeginEvent(L"Draw Cup Model");
    m_model->Draw(context, *m_states, m_world, m_view, m_proj);
    m_deviceResources->PIXEndEvent();
    //Texture room
    m_room->Draw(Matrix::Identity, m_view, m_proj,m_roomColor, m_roomTex.Get());









    //Mutlisampling
    context->ResolveSubresource(m_deviceResources->GetRenderTarget(), 0,
        m_offscreenRenderTarget.Get(), 0,
        m_deviceResources->GetBackBufferFormat());

    m_deviceResources->PIXEndEvent();
    // Show the new frame.
    m_deviceResources->Present();

}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    
    /*auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView(); No Multisampling*/
    
    //Multisampling
    auto renderTarget = m_offscreenRenderTargetSRV.Get();
    auto depthStencil = m_depthStencilSRV.Get();
    
    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto const viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion



#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    m_keys.Reset();
	m_mouseButtons.Reset();
	// TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_keys.Reset();
	m_mouseButtons.Reset();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    auto size = m_deviceResources->GetOutputSize();
    m_screenPos.x = float(size.right) / 2.f;
    m_screenPos.y = float(size.bottom) / 2.f;
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_spriteBatch = std::make_unique<SpriteBatch>(context);
    
   ComPtr<ID3D11Resource> resource;


    m_states = std::make_unique<CommonStates>(device);

    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"sunset.jpg", nullptr,
            m_background.ReleaseAndGetAddressOf()));



    //3D object 
    m_shape = GeometricPrimitive::CreateSphere(context);

    m_world = Matrix::Identity;//The coordinates of the object (Origin)

	/*3D shapes 
    m_shape = GeometricPrimitive::CreateCone(context);
    m_shape = GeometricPrimitive::CreateSphere(context);
    m_shape = GeometricPrimitive::CreateCube(context);
    m_shape = GeometricPrimitive::CreateCylinder(context);

    m_shape = GeometricPrimitive::CreateDodecahedron(context);

    m_shape = GeometricPrimitive::CreateTeapot(context);
    m_shape = GeometricPrimitive::CreateTorus(context);
    
    
    */
    
    DX::ThrowIfFailed(
        CreateWICTextureFromFile(device, L"earth.bmp", nullptr,
            m_texture.ReleaseAndGetAddressOf()));
    //Grid 

    m_world = Matrix::Identity;

    m_states = std::make_unique<CommonStates>(device);

    m_effect = std::make_unique<BasicEffect>(device);
    m_effect->SetVertexColorEnabled(true);

    DX::ThrowIfFailed(
        CreateInputLayoutFromEffect<VertexPositionColor>(device, m_effect.get(),
            m_inputLayout.ReleaseAndGetAddressOf())
    );

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

    //Anti-aliased lines
    CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
        D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE, FALSE);

    DX::ThrowIfFailed(device->CreateRasterizerState(&rastDesc,
        m_raster.ReleaseAndGetAddressOf()));


    //Modeling
    m_states = std::make_unique<CommonStates>(device);

	m_fxFactory = std::make_unique<EffectFactory>(device);


	m_model = Model::CreateFromCMO(device, L"cup.cmo", *m_fxFactory);
	m_world = Matrix::Identity;

	//Texture    
	m_room = GeometricPrimitive::CreateBox(context,
	    XMFLOAT3(ROOM_BOUNDS[0], ROOM_BOUNDS[1], ROOM_BOUNDS[2]),
	    false, true);

	DX::ThrowIfFailed(
	    CreateDDSTextureFromFile(device, L"roomtexture.dds",
	        nullptr, m_roomTex.ReleaseAndGetAddressOf()));
  
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    m_fullscreenRect = m_deviceResources->GetOutputSize();

    auto size = m_deviceResources->GetOutputSize();

    Matrix proj = Matrix::CreateScale(2.f / float(size.right), -2.f / float(size.bottom), 1.f) * Matrix::CreateTranslation(-1.f, 1.f, 0.f);
    m_effect->SetProjection(proj);

    //3D object 
    //m_view = Matrix::CreateLookAt(Vector3(3.f, 3.f, 3.f), Vector3::Zero, Vector3::UnitY);
    //m_proj = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,float(size.right) / float(size.bottom), 0.1f, 10.f);
    
	m_proj = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(70.f),float(size.right) / float(size.bottom), 0.01f, 100.f);



    //Multisampling
    auto device = m_deviceResources->GetD3DDevice();
    auto width = static_cast<UINT>(size.right);
    auto height = static_cast<UINT>(size.bottom);

    CD3D11_TEXTURE2D_DESC rtDesc(m_deviceResources->GetBackBufferFormat(),
        width, height, 1, 1,
        D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, 0,
        MSAA_COUNT, MSAA_QUALITY);

    DX::ThrowIfFailed(
        device->CreateTexture2D(&rtDesc, nullptr,
            m_offscreenRenderTarget.ReleaseAndGetAddressOf()));

    CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);

    DX::ThrowIfFailed(
        device->CreateRenderTargetView(m_offscreenRenderTarget.Get(),
            &rtvDesc,
            m_offscreenRenderTargetSRV.ReleaseAndGetAddressOf()));

    CD3D11_TEXTURE2D_DESC dsDesc(DXGI_FORMAT_D32_FLOAT,
        width, height, 1, 1,
        D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, 0,
        MSAA_COUNT, MSAA_QUALITY);

    ComPtr<ID3D11Texture2D> depthBuffer;
    DX::ThrowIfFailed(
        device->CreateTexture2D(&dsDesc, nullptr, depthBuffer.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);

    DX::ThrowIfFailed(
        device->CreateDepthStencilView(depthBuffer.Get(),
            &dsvDesc,
            m_depthStencilSRV.ReleaseAndGetAddressOf()));

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);

	
}

void Game::OnDeviceLost()
{
    m_texture.Reset();
    m_spriteBatch.reset();
    m_background.Reset();


    //for Triangle 
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_inputLayout.Reset();

    //3D object 
    m_shape.reset();
    //Grid
    
    m_inputLayout.Reset();
    m_raster.Reset();
    //Multisampling
    m_offscreenRenderTarget.Reset();
    m_offscreenRenderTargetSRV.Reset();
    m_depthStencilSRV.Reset();

    //Modeling
    m_states.reset();
	m_fxFactory.reset();
	m_model.reset();

    //Texture
    m_room.reset();
	m_roomTex.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
