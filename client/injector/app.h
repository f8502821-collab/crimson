#pragma once
// Injector UI application: hero brand, process card, the ONE inject button.
namespace injector_app {

void init();      // after ImGui backend init, before first frame
void draw();      // every frame
void shutdown();  // join worker threads

} // namespace injector_app
