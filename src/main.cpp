#include "stdafx.h"

#include "core/application.h"
#include "utils/string_utils.h"

#include <exception>
#include <iostream>
#include <string>

/// <summary>
/// Entry point for the DX12 Path Tracer application
/// <para></para>
/// Simple and clean - all complexity is encapsulated in Application class.
/// </summary>
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    try
    {
        // Create application with window size and title
        pathtracer::Application app;

        // Run the application (blocks until window closes)
        return app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        MessageBox(nullptr, pathtracer::utils::StringToWide(e.what()).c_str(),
                   TEXT("Fatal Error"), MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        MessageBox(nullptr, TEXT("Unknown fatal error occurred"),
                   TEXT("Fatal Error"), MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }
}
