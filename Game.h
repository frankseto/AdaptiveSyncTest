//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#pragma once

#include "DeviceResources.h"
#include "Basicmath.h"
#include "Sensor.h"
#include <map>

struct rawOutputDesc
{
	float MaxLuminance;
	float MaxFullFrameLuminance;
	float MinLuminance;
};

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game : public DX::IDeviceNotify
{
private:
    enum TestingTier
    {
        DisplayHDR400   = 0,
		DisplayHDR500   = 1,
		DisplayHDR600   = 2,
        DisplayHDR1000  = 3,
        DisplayHDR1400  = 4,
		DisplayHDR2000  = 5,
		DisplayHDR3000  = 6,
        DisplayHDR4000  = 7,
        DisplayHDR6000  = 8,
        DisplayHDR10000 = 9
    };

    enum ColorGamut
    {
        GAMUT_Native = 0,
        GAMUT_sRGB   = 1,
        GAMUT_Adobe  = 2,
        GAMUT_DCIP3  = 3,
        GAMUT_BT2100 = 4,
        GAMUT_ACES   = 5,
    };

    // each test indexes into these arrays using its currentRateIndex
#define numFlickerRates 5

#define numMediaRates 13
    float mediaFrameRates[numMediaRates] = { 23.976f, 24.0f, 25.0f, 29.97f, 30.0f, 47.952f, 48.0f, 50.0f, 59.94f, 60.0f, 120.0f, 180.0f, 240.0f };

#define numFrameDropRates 8
    float frameDropRates[numFrameDropRates] = { 24., 30.f, 48.f, 60.f, 90.f, 120.f, 180.f, 240.f };

#define maxSleepDelay 120.          // ms

#define numGtGValues 5

#define numCommonRates 10
    float commonFrameRates[numCommonRates] = { 30.f, 48.f, 60.f, 90.f, 120.f, 144.f, 180.f, 240.f, 300.f, 360.f };

    const INT32 maxMotionBlurs = 10;

#define frameLogLength 11
    typedef struct frameEventStruct {
        UINT64 frameID;			    // frame counter value for this frame
        INT64 clickCounts;	    	// count when input device button was clicked
        INT64 readCounts;	    	// count when frame processing starts in app
        INT64 drawCounts;	    	// count when drawing starts (GPU rendering)
        INT64 presentCounts;   	    // count when app is done with rendering -Present() is called
        INT64 syncCounts;	    	// count when GPU video system starts sending this image aka time of the Flip/Sync
        INT64 photonCounts;		    // count at point when photons detected by sensor
    } FrameEvents;

    FrameEvents *m_frameLog[frameLogLength];            // array of pointers to event structs
    FrameEvents m_frameEvents[frameLogLength];          // storage for all the frame event structs

    // Used by any test that loads a custom effect or image from file.
    struct TestPatternResources
    {
        std::wstring testTitle; // Mandatory.
        std::wstring imageFilename; // Empty means no image is needed for this test.
        std::wstring effectShaderFilename; // Empty means no shader is needed for this test.
        GUID effectClsid;
        // Members above this point need to be specified at app start.
        // ---
        // Members below this point are generated dynamically.
        Microsoft::WRL::ComPtr<IWICBitmapSource> wicSource; // Generated from WIC.
        Microsoft::WRL::ComPtr<ID2D1ImageSourceFromWic> d2dSource; // Generated from D2D.
        Microsoft::WRL::ComPtr<ID2D1Effect> d2dEffect; // Generated from D2D.
        bool imageIsValid; // false means image file is missing or invalid.
        bool effectIsValid; // false means effect file is missing or invalid.
    };

public:

    Game(PWSTR appTitle);

    enum class WaveEnum
    {
        Nil,
        ZigZag,
        SquareWave,
        Random,
        Sine,
        Max,
    };

	enum class TestPattern
	{
		StartOfTest,                // Must always be first.
		ConnectionProperties,       //      Can we set resolution of fullscreen mode here?
		PanelCharacteristics,       //  1   report limits of frame rate possible at this resolution
		ResetInstructions,
        FlickerConstant,            //  2   MinHz, MaxHz, 60Hz, and 24/48Hz
        FlickerVariable,            //  3   Sine wave vs square wave    should use this for frame drop test too
        DisplayLatency,             //  4   Measure e2e lag at 60Hz, 90, 120, 180, 240, 300, 360, 420, 480
        GrayToGray,                 //  5   Scene to measure gray-to-gray response time
        FrameDrop,                  //  6   Draw the animated square. 60:10x6, 90:10x9, 120:12x10, 180:15x12, 240:16x15, 
        FrameLock,                  //  7   Select from 23.976, 24, 25, 29.97, 30, 47.952, 48, 50, 59.94, 60Hz -Media Jitter test
        EndOfMandatoryTests,        // 
        MotionBlur,                 //  8   Demonstrate correct motion blur vs panel exposure time (frameFraction)
        GameJudder,                 //  9   VRR scrolling image to minimize display duration per game needs (BFI)
        Tearing,                    //      Test pattern to show off tearing artifacts at rates outside the valid range.
        EndOfTest,                  // Must always be last.
        WarmUp,                     //  W
        Cooldown,                   // 'C'   on C hotkey
    };

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();
    void TickOld();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);
    void OnDisplayChange();

    // Properties
    void GetDefaultSize(int& width, int& height) const;

    // Test pattern control
    void SetTestPattern(TestPattern testPattern);
    void ChangeTestPattern(bool increment);
    void ChangeSubtest(bool increment);
    void ChangeG2GFromIndex(bool increment);
    void ChangeG2GToIndex(bool increment);
    void StartTestPattern(void);
    void TogglePause(void);
    void ToggleSensing(void);
    void ResetCurrentStats(void);                     // reset whichever stats are currently in use

    void ChangeGradientColor(float deltaR, float deltaG, float deltaB);
    void ChangeBackBufferFormat(DXGI_FORMAT fmt);
    bool ToggleInfoTextVisible();
    void SetMetadataNeutral(); // OS defaults
	void PrintMetadata( ID2D1DeviceContext2* ctx, bool blackText = false );

 protected:
    void CheckTearingSupport();
    bool                                                    m_tearingSupport;           // Whether or not tearing is available for fullscreen borderless windowed mode.

private:

    void ConstructorInternal();

    void Update();
    void UpdateDxgiColorimetryInfo();
    void UpdateDxgiRefreshRatesInfo();
    void InitEffectiveValues();
    void SetMetadata(float max, float avg, ColorGamut gamut);
    void Render();
	bool CheckHDR_On();
    bool CheckForDefaults();
	void DrawLogo(ID2D1DeviceContext2 *ctx, float c );
    TestingTier GetTestingTier();
    WCHAR *GetTierName(TestingTier tier);
	float GetTierLuminance(Game::TestingTier tier);
    bool isMedia();                                             // whether this test uses media fixed rates or game dynamic rates
    void RotateFrameLog();                                      // Shuffle entries down to make room
    float GrayToGrayValue(INT32 index);

    void ResetSensorStats(void);
    void ResetFrameStats(void);

    // float ComputeGamutArea( float2 r, float2 g, float2 b );
    // float ComputeGamutCoverage( float2 r1, float2 g1, float2 b1, float2 r2, float2 g2, float2 b2 );

    // Drawing code specific for each test pattern.
    void GenerateTestPattern_StartOfTest(ID2D1DeviceContext2* ctx);
    void GenerateTestPattern_ConnectionProperties(ID2D1DeviceContext2* ctx);
    void GenerateTestPattern_PanelCharacteristics(ID2D1DeviceContext2* ctx);            // 1
    void GenerateTestPattern_ResetInstructions(ID2D1DeviceContext2* ctx);
	void GenerateTestPattern_FlickerConstant(ID2D1DeviceContext2* ctx);                 // 2
	void GenerateTestPattern_FlickerVariable(ID2D1DeviceContext2* ctx);                 // 3
	void GenerateTestPattern_DisplayLatency(ID2D1DeviceContext2* ctx);                  // 4
	void GenerateTestPattern_GrayToGray(ID2D1DeviceContext2* ctx);                      // 5
    void GenerateTestPattern_FrameDrop(ID2D1DeviceContext2* ctx);                       // 6
    void GenerateTestPattern_FrameLock(ID2D1DeviceContext2* ctx);                       // 7
    void GenerateTestPattern_EndOfMandatoryTests(ID2D1DeviceContext2* ctx);
    void GenerateTestPattern_MotionBlur(ID2D1DeviceContext2* ctx);						// 8
    void GenerateTestPattern_GameJudder(ID2D1DeviceContext2* ctx);                      // 9
    void GenerateTestPattern_Tearing(ID2D1DeviceContext2* ctx);                         // T
    void GenerateTestPattern_EndOfTest(ID2D1DeviceContext2* ctx);
    void GenerateTestPattern_WarmUp(ID2D1DeviceContext2* ctx);                          // W
    void GenerateTestPattern_Cooldown(ID2D1DeviceContext2* ctx);                        // C

    // Generalized routine for all tests that involve loading an image.
    void GenerateTestPattern_ImageCommon(ID2D1DeviceContext2* ctx, TestPatternResources resources);

    // Common rendering subroutines.
    void Clear();
    void RenderText(ID2D1DeviceContext2* ctx, IDWriteTextFormat* fmt, std::wstring text, D2D1_RECT_F textPos, bool useBlackText = false);

    void CreateDeviceIndependentResources();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void LoadTestPatternResources(TestPatternResources* resources);
    void LoadImageResources(TestPatternResources* resources);
    void LoadEffectResources(TestPatternResources* resources);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>        m_gradientBrush;
    Microsoft::WRL::ComPtr<IDWriteTextLayout>               m_testTitleLayout;
    Microsoft::WRL::ComPtr<IDWriteTextLayout>               m_panelInfoTextLayout;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>            m_whiteBrush;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>            m_blackBrush;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>            m_redBrush;

    DXGI_OUTPUT_DESC1                                       m_outputDesc;
	rawOutputDesc											m_rawOutDesc;		// base values from OS before scaling due to brightness setting

    // Device independent resources.
    Microsoft::WRL::ComPtr<IDWriteTextFormat>               m_smallFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat>               m_largeFormat;
    Microsoft::WRL::ComPtr<IDWriteTextFormat>               m_monospaceFormat;
    D2D1_RECT_F                 m_testTitleRect; // Where to draw each test's title
    D2D1_RECT_F                 m_largeTextRect; // Where to draw large text for the test, if applicable
	D2D1_RECT_F					m_MetadataTextRect;
    TestingTier                 m_testingTier;
    TestPattern                 m_currentTest;
    TestPattern                 m_cachedTest;

    INT32                       m_modeWidth;                // resolution of current mode (actually native res now)
    INT32                       m_modeHeight;

    LARGE_INTEGER               m_qpcFrequency;             // clock frequency on this PC
    INT64                       m_lastReadCounts;           // qpc counts from when we started working on last frame
    double                      m_sleepDelay;               // ms simulate workload of app (just used for some tests)
    INT32                       m_frameCount;               // number of frames in average
    double                      m_frameTime;                // total time since last frame start in seconds
    double                      m_lastFrameTime;            // save from one frame ago
    double                      m_totalFrameTime;           // for average frame time
    double                      m_totalFrameTime2;          // sum of squares of above for variance
    double                      m_totalRenderTime;          // for average Render time
    double                      m_totalRenderTime2;         // sum of squares of above for variance
    double                      m_totalPresentTime;         // for average Present time
    double                      m_totalPresentTime2;        // sum of squares of above for variance
    double                      m_avgInputTime;             // hard-coded until dongle drives input

    double                      m_totalTimeSinceStart;      // not sure if I need this?
    double                      m_testTimeRemainingSec;     // how long current test has been running in seconds
    uint64_t                    m_frameCounter;             // how many frames rendered in app
    double                      m_targetFrameRate;          // frame rate we want to achieve               
    bool                        m_paused;                   // whether we are updating data or not

    double                      m_maxFrameRate;             // maximum this device can support                  1
    double                      m_minFrameRate;             // minimum this device can support
    double                      m_FrameRateRatio;           // ratio of above 2 parameters

    INT32                       m_flickerRateIndex;         // select frame rate in flicker test                2

    INT32                       m_waveCounter;              // control for square wave                          3
    WaveEnum                    m_waveEnum;                 // zigzag vs square wave vs random                  3
    bool                        m_waveUp;                   // zig up or down
    INT32                       m_latencyRateIndex;         // select frame rate in frame latency test          4
    double                      m_latencyTestFrameRate;     // frame rate specific to this test pattern         4
    bool                        m_sensorConnected;          // connection to the photocell dongle is live       4
    bool                        m_sensing;                  // whether we are running the sensor                4
    bool                        m_flash;                    // whether we are flashing the photocell this frame 4
    double                      m_minSensorTime;            // min value of end-to-end lag measured             4
    double                      m_maxSensorTime;            // max value of end-to-end lag measured             4
    INT32                       m_sensorCount;              // number of valid latency samples since reset      4
    double                      m_totalSensorTime;          // sum of sensor time. Used to compute average      4
    double                      m_totalSensorTime2;         // sum of squares. Used to compute variance         4

    INT32                       m_g2gFromIndex;             // counter for the GtG level to transition from     5
    INT32                       m_g2gToIndex;               // counter for the GtG level we transition to       5

    INT32                       m_frameDropRateIndex;       // select frame rate in frame drop test             6

    INT32                       m_frameLockRateIndex;       // select frame rate in frame lock test             7
    INT32                       m_mediaRateIndex;           // ??
    float                       m_fAngle;                   // angle where moving object is at                  8
    INT32                       m_MotionBlurIndex;          // counter for frame fraction                       8
//  bool                        m_bMotionBlur;              // whether Motion Blur is on                        8
    double                      m_judderTestFrameRate;      // for BFI test

    float                       m_sweepPos;                 // position of bar in tearing test (pixels)         0
    double                      m_tearingTestFrameRate;     // for tearing check

    INT32						m_currentProfileTile;
	UINT32						m_maxPQCode;		        // PQ code of maxLuminance
	INT32						m_maxProfileTile;	        // highest tile worth testing on this panel
    bool                        m_showExplanatoryText;
	bool                        m_newTestSelected;          // Used for one-time initialization of test variables.
    bool                        m_dxgiColorInfoStale;
	DXGI_HDR_METADATA_HDR10		m_Metadata;
	ColorGamut					m_MetadataGamut;


    // TODO: integrate this with the other test resources
    std::map<TestPattern, TestPatternResources>             m_testPatternResources;
    std::wstring                                            m_hideTextString;

    PWSTR                                                   m_appTitle;
};