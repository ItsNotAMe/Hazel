#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_WARN("Initialized Log!");
	int a = 42;
	HZ_INFO("Hello! VAR={0}", a);

	auto app = Hazel::CreateApplication();
	app->Run();
	delete app;
}

#endif