#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define DEBUG_FASTLOAD false
#define baseScreenWidth 1920
#define baseScreenHeight 1080

const float baseX = -(baseScreenWidth / 2);
const float baseY = -(baseScreenHeight / 2);
const float targetAspectRatio = (float)baseScreenWidth / (float)baseScreenHeight;
const int targetFps = 300;

Texture2D logoTexture;
Texture2D splashBackgroundTexture;
Texture2D splashOverlayTexture;
Texture2D backgroundTexture;
Texture2D backgroundOverlayTexture;
Texture2D backgroundOverlaySidebarTexture;
Texture2D pawTexture;
Font meowFont;
Sound select;
Sound hover;

Texture2D mainMenuFallingItems[8];

typedef enum {
    EASY,
    MEDIUM,
    HARD
} Difficulty;

static inline char* StringFromDifficultyEnum(Difficulty difficulty)
{
    static const char* strings[] = { "Easy", "Medium", "Hard" };
    return strings[difficulty];
}

typedef struct Resolution {
	int x;
	int y;
} Resolution;

// Game Options in struct
typedef struct GameOptions {
    Resolution resolution;
	int targetFps;
	bool fullscreen;
    Difficulty difficulty;
} GameOptions;

GameOptions *options;

int GetRandomIntValue(int min, int max)
{
	return min + rand() % (max - min + 1);
}

void SetRuntimeResolution(Camera2D *camera, int screenWidth, int screenHeight)
{
    SetWindowSize(screenWidth, screenHeight);
    float currentAspectRatio = (float)screenWidth / (float)screenHeight;
    float scale = (currentAspectRatio > targetAspectRatio) ? (float)screenHeight / baseScreenHeight : (float)screenWidth / baseScreenWidth;

    camera->zoom = scale;
    camera->offset.x = screenWidth / 2.0f;
    camera->offset.y = screenHeight / 2.0f;

    options->resolution.x = screenWidth;
    options->resolution.y = screenHeight;
}

// Function prototype
void MainMenuUpdate(Camera2D* camera, bool playFade);
void OptionsUpdate(Camera2D* camera);

void WindowUpdate(Camera2D* camera)
{
    if (IsWindowResized())
    {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();


        float currentAspectRatio = (float)screenWidth / (float)screenHeight;
        float scale = (currentAspectRatio > targetAspectRatio) ? (float)screenHeight / baseScreenHeight : (float)screenWidth / baseScreenWidth;

        camera->zoom = scale;
        camera->offset.x = screenWidth / 2.0f;
        camera->offset.y = screenHeight / 2.0f;

        options->resolution.x = screenWidth;
        options->resolution.y = screenHeight;
    }

    // Alt - Enter fullscreen
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER) || IsKeyDown(KEY_RIGHT_ALT) && IsKeyPressed(KEY_ENTER))
    {
        SetRuntimeResolution(camera, baseScreenWidth, baseScreenHeight);
        ToggleFullscreen();
    }
}

void LoadGlobalAssets()
{
    backgroundTexture = LoadTexture(ASSETS_PATH"image/backgrounds/main.png");
    backgroundOverlayTexture = LoadTexture(ASSETS_PATH"image/backgrounds/main_overlay_1.png");
    backgroundOverlaySidebarTexture = LoadTexture(ASSETS_PATH"image/backgrounds/main_overlay_2.png");
    pawTexture = LoadTexture(ASSETS_PATH"image/elements/paw.png");
    meowFont = LoadFontEx(ASSETS_PATH"font/Meows-VGWjy.ttf", 256, 0, 250);

    hover = LoadSound(ASSETS_PATH"audio/hover.wav");
    select = LoadSound(ASSETS_PATH"audio/select.wav");

    mainMenuFallingItems[0] = LoadTexture(ASSETS_PATH"image/falling_items/cara.png");
    mainMenuFallingItems[1] = LoadTexture(ASSETS_PATH"image/falling_items/cmilk.png");
    mainMenuFallingItems[2] = LoadTexture(ASSETS_PATH"image/falling_items/cocoa.png");
    mainMenuFallingItems[3] = LoadTexture(ASSETS_PATH"image/falling_items/gar.png");
    mainMenuFallingItems[4] = LoadTexture(ASSETS_PATH"image/falling_items/marshmello.png");
    mainMenuFallingItems[5] = LoadTexture(ASSETS_PATH"image/falling_items/matcha.png");
    mainMenuFallingItems[6] = LoadTexture(ASSETS_PATH"image/falling_items/milk.png");
    mainMenuFallingItems[7] = LoadTexture(ASSETS_PATH"image/falling_items/wcream.png");
}

void UnloadGlobalAssets()
{
    UnloadTexture(backgroundTexture);
    UnloadTexture(backgroundOverlayTexture);
    UnloadTexture(backgroundOverlaySidebarTexture);
    UnloadTexture(pawTexture);
    UnloadFont(meowFont);

    UnloadSound(hover);
    UnloadSound(select);

    for(int i = 0; i < 8; i++)
		UnloadTexture(mainMenuFallingItems[i]);
}

void ExitApplication()
{
    UnloadGlobalAssets();
    
    // Exit with exit code 0
    exit(0);
}


void OptionsUpdate(Camera2D* camera)
{
    // Define rectangles for UI elements (e.g., buttons, dropdowns)
    Rectangle difficultyRect = { baseX + 100, baseY + 30, 200, 50 };
    Rectangle resolutionRect = { baseX + 100, baseY + 120, 200, 50 };
    Rectangle fpsRect = { baseX + 100, baseY + 220, 200, 50 };
    Rectangle fullscreenRect = { baseX + 100, baseY + 320, 200, 50 };
    Rectangle backRect = { baseX + 100, baseY + 480, 200, 50 };

    bool firstRender = true;

    while (!WindowShouldClose())
    {
        WindowUpdate(camera);

        // Convert mouse position from screen space to world space
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

        bool isDifficultySelected = CheckCollisionPointRec(mouseWorldPos, difficultyRect);
        bool isResolutionSelected = CheckCollisionPointRec(mouseWorldPos, resolutionRect);
        bool isFpsSelected = CheckCollisionPointRec(mouseWorldPos, fpsRect);
        bool isFullscreenToggled = CheckCollisionPointRec(mouseWorldPos, fullscreenRect);
        bool isBackSelected = CheckCollisionPointRec(mouseWorldPos, backRect);

        // Handle user input
        if (!firstRender && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (isDifficultySelected) {
                // Toggle difficulty (cycle through the options)
                options->difficulty = (options->difficulty + 1) % 3;
            }
            else if (isResolutionSelected) {
                // 1280x720, 1920x1080
                if (options->resolution.x == 1920) {
                    options->resolution.x = 1280;
                    options->resolution.y = 720;
                }
                else if (options->resolution.x == 1280) {
					options->resolution.x = 1920;
					options->resolution.y = 1080;
				}
                else
                {
                    // Custom resize resolution
                    // Change it to closest resolution, either 1280x720, 1920x1080
                    if (options->resolution.x > 1600) {
						options->resolution.x = 1920;
						options->resolution.y = 1080;
					}
					else {
						options->resolution.x = 1280;
						options->resolution.y = 720;
					}

                }
                SetRuntimeResolution(camera, options->resolution.x, options->resolution.y);
            }
            else if (isFpsSelected) {
                // Change target FPS 30, 60, 120, 144, 240, Basically Unlimited (1000)
                if (options->targetFps == 30) {
					options->targetFps = 60;
				}
				else if (options->targetFps == 60) {
					options->targetFps = 120;
				}
				else if (options->targetFps == 120) {
					options->targetFps = 144;
				}
				else if (options->targetFps == 144) {
					options->targetFps = 240;
				}
				else if (options->targetFps == 240) {
					options->targetFps = 1000;
				}
				else if (options->targetFps == 1000) {
					options->targetFps = 30;
				}

                SetTargetFPS(options->targetFps);
            }
            else if (isFullscreenToggled) {
                // Toggle fullscreen
                options->fullscreen = !options->fullscreen;
                SetRuntimeResolution(camera, options->resolution.x, options->resolution.y);
                ToggleFullscreen();
            }
            else if (isBackSelected) {
				// Go back to main menu
				MainMenuUpdate(camera, false);
				break;
			}
        }

        if (firstRender)
            firstRender = false;

        // Draw

        BeginDrawing();
        ClearBackground(BLACK);


        BeginMode2D(*camera);


        int imageWidth = backgroundTexture.width;
        int imageHeight = backgroundTexture.height;

        float scaleX = (float)baseScreenWidth / imageWidth;
        float scaleY = (float)baseScreenHeight / imageHeight;

        // Draw the background with the scaled dimensions
        DrawTextureEx(backgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);

        // Draw UI elements
        DrawRectangleRec(difficultyRect, isDifficultySelected ? SKYBLUE : BLUE);
        DrawRectangleRec(resolutionRect, isResolutionSelected ? SKYBLUE : BLUE);
        DrawRectangleRec(fpsRect, isFpsSelected ? SKYBLUE : BLUE);
        DrawRectangleRec(fullscreenRect, isFullscreenToggled ? SKYBLUE : BLUE);
        DrawRectangleRec(backRect, isBackSelected ? SKYBLUE : BLUE);

        char difficultyText[50];
        sprintf(difficultyText, "Difficulty: %s", StringFromDifficultyEnum(options->difficulty));

        char targetFpsText[20];
        sprintf(targetFpsText, "Fps: %d", options->targetFps);

        char resolutionText[20];
        sprintf(resolutionText, "%dx%d", options->resolution.x, options->resolution.y);

        DrawText(difficultyText, (int)(difficultyRect.x + 40), (int)(difficultyRect.y + 15), 20, WHITE);
        DrawText(resolutionText, (int)(resolutionRect.x + 40), (int)(resolutionRect.y + 15), 20, WHITE);
        DrawText(targetFpsText, (int)(fpsRect.x + 40), (int)(fpsRect.y + 15), 20, WHITE);
        DrawText("Fullscreen", (int)(fullscreenRect.x + 40), (int)(fullscreenRect.y + 15), 20, WHITE);
        DrawText("Back", (int)(backRect.x + 40), (int)(backRect.y + 15), 20, WHITE);

        EndMode2D();
        EndDrawing();
    }

    UnloadTexture(backgroundTexture);
    CloseWindow();
}

void GameUpdate(Camera2D *camera)
{
    float circleRadius = 30.0f;
    Vector2 circlePosition = { 100, 50 };
    bool isDragging = false;

    while (!WindowShouldClose())
    {
        WindowUpdate(camera);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            //Vector2 mousePos = GetMousePosition();

            // Convert mouse position to world space, taking into account camera zoom
            //mousePos.x = (mousePos.x - GetScreenWidth() / 2.0f) / camera->zoom + camera->target.x;
            //mousePos.y = (mousePos.y - GetScreenHeight() / 2.0f) / camera->zoom + camera->target.y;

            // Convert mouse position from screen space to world space
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

            // Calculate the vector from the circle's center to the mouse position
            Vector2 circleToMouse = Vector2Subtract(mouseWorldPos, circlePosition);

            // Check if the mouse click is within the circle (distance <= circleRadius)
            if (Vector2Length(circleToMouse) <= circleRadius)
            {
                isDragging = true;
            }
        }
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            isDragging = false;
        }

        if (isDragging)
        {
            // Update circle position during dragging, considering camera zoom
            circlePosition = GetMousePosition();
            circlePosition.x = (circlePosition.x - GetScreenWidth() / 2.0f) / camera->zoom + camera->target.x;
            circlePosition.y = (circlePosition.y - GetScreenHeight() / 2.0f) / camera->zoom + camera->target.y;
        }


        // Draw

        BeginDrawing();
        ClearBackground(BLACK);


        BeginMode2D(*camera);


        int imageWidth = backgroundTexture.width;
        int imageHeight = backgroundTexture.height;

        float scaleX = (float)baseScreenWidth / imageWidth;
        float scaleY = (float)baseScreenHeight / imageHeight;

        // Draw the background with the scaled dimensions
        DrawTextureEx(backgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);


        // Draw circle
        DrawCircleV(circlePosition, circleRadius, BLUE);

        // Text
        DrawTextEx(GetFontDefault(), "Hello, Test!", (Vector2) { baseX + 20, baseY + 20 }, 20, 2, WHITE);

        EndMode2D();

        EndDrawing();
    }

    UnloadTexture(backgroundTexture);
    CloseWindow();

}

typedef struct {
    Vector2 position;
    float rotation;
    int textureIndex;
    float fallingSpeed;
    float rotationSpeed;
} FallingItem;

void MainMenuUpdate(Camera2D* camera, bool playFade)
{
    float fadeOutDuration = 1.0f;
    double currentTime = 0;
    double lastFrameTime = GetTime();

    int splashBackgroundImageWidth = splashBackgroundTexture.width;
    int splashBackgroundImageHeight = splashBackgroundTexture.height;

    float splashBackgroundScaleX = (float)baseScreenWidth / splashBackgroundImageWidth;
    float splashBackgroundScaleY = (float)baseScreenHeight / splashBackgroundImageHeight;

    float transitionOffset = 0;

    bool isFadeOutDone = false;
    bool isTransitioningIn = !playFade;
    bool isTransitioningOut = false;

    if(isTransitioningIn)
        transitionOffset = baseScreenWidth / 2;

    const int numItems = 8; // The number of different falling items

    FallingItem fallingItems[20];

    for (int i = 0; i < 20; i++) {
        fallingItems[i].position = (Vector2){ GetRandomIntValue(baseX, baseX + baseScreenWidth - 20), baseY - GetRandomValue(200, 1000) };
        fallingItems[i].textureIndex = GetRandomIntValue(0, numItems - 1);

        // Random rotation and falling speed
        fallingItems[i].rotation = GetRandomIntValue(-360, 360);
        fallingItems[i].fallingSpeed = GetRandomIntValue(1, 3);
        fallingItems[i].fallingSpeed *= 100;

        // Rotation speed is between -3 and 3, but should not execeed falling speed and should not be 0
        fallingItems[i].rotationSpeed = GetRandomIntValue(-3, 3);
        if (abs(fallingItems[i].rotationSpeed) > fallingItems[i].fallingSpeed)
            fallingItems[i].rotationSpeed = fallingItems[i].fallingSpeed;
        if (fallingItems[i].rotationSpeed == 0)
            fallingItems[i].rotationSpeed = 1;

        fallingItems[i].rotationSpeed *= 100;

    }

    int currentHoveredButton = NULL;
    void (*transitionCallback)(Camera2D * camera) = NULL;

    bool isHovering = false;

    while (!WindowShouldClose())
    {
        // Calculate delta time
        double deltaTime = GetTime() - lastFrameTime;
        lastFrameTime = GetTime();

        if (!isFadeOutDone)
            currentTime += GetFrameTime();

        WindowUpdate(camera);

        // Button positions and dimensions
        Rectangle startButtonRect = { baseX + 50 - transitionOffset, baseY + 600 + 60, 400, 100 };
        Rectangle optionsButtonRect = { baseX + 50 - transitionOffset, baseY + 600 + 160, 400, 100 };
        Rectangle exitButtonRect = { baseX + 50 - transitionOffset, baseY + 600 + 350, 400, 100 };

        Rectangle buttons[] = { startButtonRect, optionsButtonRect, exitButtonRect };

        // Convert mouse position from screen space to world space
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

        bool isStartButtonHovered = CheckCollisionPointRec(mouseWorldPos, startButtonRect);
        bool isOptionsButtonHovered = CheckCollisionPointRec(mouseWorldPos, optionsButtonRect);
        bool isExitButtonHovered = CheckCollisionPointRec(mouseWorldPos, exitButtonRect);

        // Check for button clicks
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isTransitioningOut && !isTransitioningIn)
        {
            if (isStartButtonHovered)
            {
                PlaySound(select);
                transitionCallback = GameUpdate;
                isTransitioningOut = true;
            }
            else if (isOptionsButtonHovered)
            {
                PlaySound(select);
                transitionCallback = OptionsUpdate;
                isTransitioningOut = true;
            }
            else if (isExitButtonHovered)
            {
                ExitApplication();
            }
        }

        // Play sound when hovering over a button, but only once
        if (isStartButtonHovered)
        {
            if (!isHovering || currentHoveredButton != 0)
            {
                PlaySound(hover);
                isHovering = true;
            }
            currentHoveredButton = 0;
        }
        else if (isOptionsButtonHovered)
        {
            if (!isHovering || currentHoveredButton != 1)
            {
                PlaySound(hover);
                isHovering = true;
            }
            currentHoveredButton = 1;
        }
        else if (isExitButtonHovered)
        {
            if (!isHovering || currentHoveredButton != 2)
            {
                PlaySound(hover);
                isHovering = true;
            }
            currentHoveredButton = 2;
        }
        else
        {
            currentHoveredButton = NULL;
            isHovering = false;
        }


        // Draw

        BeginDrawing();
        BeginMode2D(*camera);
        ClearBackground(RAYWHITE);

        int imageWidth = backgroundTexture.width;
        int imageHeight = backgroundTexture.height;

        float scaleX = (float)baseScreenWidth / imageWidth;
        float scaleY = (float)baseScreenHeight / imageHeight;

        int imageLogoWidth = logoTexture.width;
        int imageLogoHeight = logoTexture.height;

        float scaleLogoX = (float)baseScreenWidth / imageLogoWidth / 4;
        float scaleLogoY = (float)baseScreenHeight / imageLogoHeight / 4;

        // Draw the background with the scaled dimensions
        DrawTextureEx(backgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);

        // Draw buttons
        /*
        DrawRectangleRec(startButtonRect, isStartButtonHovered ? SKYBLUE : BLUE);
        DrawRectangleRec(optionsButtonRect, isOptionsButtonHovered ? SKYBLUE : BLUE);
        DrawRectangleRec(exitButtonRect, isExitButtonHovered ? SKYBLUE : BLUE);
        */

        // Draw falling items
        for (int i = 0; i < 11; i++) {
            FallingItem* item = &fallingItems[i];

            // Calculate the item's position change based on its falling speed and deltaTime
            item->position.y += item->fallingSpeed * deltaTime;

            // Calculate the item's rotation change based on its rotation speed and deltaTime
            item->rotation += item->rotationSpeed * deltaTime;

            Vector2 origin = { (float)mainMenuFallingItems[item->textureIndex].width / 2, (float)mainMenuFallingItems[item->textureIndex].height / 2 };
            DrawTexturePro(mainMenuFallingItems[item->textureIndex], (Rectangle) { 0, 0, mainMenuFallingItems[item->textureIndex].width, mainMenuFallingItems[item->textureIndex].height },
                (Rectangle) {
                item->position.x, item->position.y, mainMenuFallingItems[item->textureIndex].width, mainMenuFallingItems[item->textureIndex].height
            },
                origin, item->rotation, WHITE);

            // Top left of the screen is (baseX, baseY)
            if (item->position.y > baseY + baseScreenHeight + 1000) {
                item->position = (Vector2){ GetRandomIntValue(baseX, baseX + baseScreenWidth - 20), baseY - GetRandomValue(200, 1000) };
                item->textureIndex = GetRandomIntValue(0, numItems - 1);
                item->fallingSpeed = GetRandomIntValue(1, 3);
                item->fallingSpeed *= 100;
                item->rotation = GetRandomIntValue(-360, 360);
                item->rotationSpeed = GetRandomIntValue(-3, 3);
                item->rotationSpeed *= 100;
                if (abs(item->rotationSpeed) > item->fallingSpeed)
                    item->rotationSpeed = item->fallingSpeed;
                if (item->rotationSpeed == 0)
                    item->rotationSpeed = 1 * 100;
            }
        }


        DrawTextureEx(backgroundOverlayTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);

        for (int i = 11; i < 20; i++) {
            FallingItem* item = &fallingItems[i];

            // Calculate the item's position change based on its falling speed and deltaTime
            item->position.y += item->fallingSpeed * deltaTime;

            // Calculate the item's rotation change based on its rotation speed and deltaTime
            item->rotation += item->rotationSpeed * deltaTime;

            Vector2 origin = { (float)mainMenuFallingItems[item->textureIndex].width / 2, (float)mainMenuFallingItems[item->textureIndex].height / 2 };
            DrawTexturePro(mainMenuFallingItems[item->textureIndex], (Rectangle) { 0, 0, mainMenuFallingItems[item->textureIndex].width, mainMenuFallingItems[item->textureIndex].height },
                (Rectangle) {
                item->position.x, item->position.y, mainMenuFallingItems[item->textureIndex].width, mainMenuFallingItems[item->textureIndex].height
            },
                origin, item->rotation, WHITE);

            // Top left of the screen is (baseX, baseY)
            if (item->position.y > baseY + baseScreenHeight + 1000) {
                item->position = (Vector2){ GetRandomIntValue(baseX, baseX + baseScreenWidth - 20), baseY - GetRandomIntValue(200, 1000) };
                item->textureIndex = GetRandomIntValue(0, numItems - 1);
                item->fallingSpeed = GetRandomIntValue(1, 3);
                item->fallingSpeed *= 100;
                item->rotation = GetRandomIntValue(-360, 360);
                item->rotationSpeed = GetRandomIntValue(-3, 3);
                item->rotationSpeed *= 100;
                if (abs(item->rotationSpeed) > item->fallingSpeed)
                    item->rotationSpeed = item->fallingSpeed;
                if (item->rotationSpeed == 0)
                    item->rotationSpeed = 1 * 100;
            }
        }


        // If transitioning out, move the background to the left
        if (isTransitioningOut)
        {
            float transitionSpeed = 900.0f;
            transitionOffset += transitionSpeed * deltaTime;

            if (transitionOffset >= baseScreenWidth / 2)
            {
                isTransitioningOut = false;
                transitionCallback(camera);
                break;
            }
        }

        // If transitioning in, move the background to the right
        if (isTransitioningIn)
		{
			float transitionSpeed = 900.0f;
			transitionOffset -= transitionSpeed * deltaTime;

			if (transitionOffset <= 0)
			{
				isTransitioningIn = false;
			}
		}


        // Draw area outside the view
        DrawRectangle(baseX, baseY - 2000, baseScreenWidth, 2000, (Color) { 0, 0, 0, 255 });
        DrawRectangle(baseX, baseY + baseScreenHeight, baseScreenWidth, 2000, (Color) { 0, 0, 0, 255 });
        DrawRectangle(baseX - 2000, baseY, 2000, baseScreenHeight, (Color) { 0, 0, 0, 255 });
        DrawRectangle(baseX + baseScreenWidth, baseY, 2000, baseScreenHeight, (Color) { 0, 0, 0, 255 });

        // Left sidebar white 
        DrawTextureEx(backgroundOverlaySidebarTexture, (Vector2) { baseX - transitionOffset, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);

        // Logo
        DrawTextureEx(logoTexture, (Vector2) { baseX - transitionOffset, baseY - 50 }, 0.0f, fmax(scaleLogoX, scaleLogoY), WHITE);

        // Draw button labels
        DrawTextEx(meowFont, "Start Game", (Vector2) { (int)(startButtonRect.x + 40), (int)(startButtonRect.y + 15) }, 60, 2, isStartButtonHovered ? (Color) { 150, 104, 81, 255 } : BLACK);
        DrawTextEx(meowFont, "Options", (Vector2) { (int)(optionsButtonRect.x + 40), (int)(optionsButtonRect.y + 15) }, 60, 2, isOptionsButtonHovered ? (Color) { 150, 104, 81, 255 } : BLACK);
        DrawTextEx(meowFont, "Exit", (Vector2) { (int)(exitButtonRect.x + 40), (int)(exitButtonRect.y + 15) }, 60, 2, isExitButtonHovered ? (Color) { 255, 0, 0, 255 } : BLACK);


        // Calculate alpha based on the current time
        if (playFade)
        {
            float alpha = (float)(255.0 * (1.0 - fmin(currentTime / fadeOutDuration, 1.0)));
            if (!isFadeOutDone && alpha != 0)
                DrawTextureEx(splashBackgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(splashBackgroundScaleX, splashBackgroundScaleY), (Color) { 255, 255, 255, alpha });
            else
                isFadeOutDone = true;
        }


        DrawFPS(baseX + 5, baseY + 5);

        EndMode2D();
        EndDrawing();
    }

    UnloadTexture(backgroundTexture);
    CloseWindow();
}

void SplashUpdate(Camera2D* camera)
{
#if DEBUG_FASTLOAD
    const double beforeStart = 0;
    const double splashDuration = 0;
    const double fadeInDuration = 0;
    const double stayDuration = 0;
    const double fadeOutDuration = 0;
    const double afterEnd = 0;
#else
    const double beforeStart = 1.0;
    const double splashDuration = 8.0;
    const double fadeInDuration = 2.0;
    const double stayDuration = 3.0;
    const double fadeOutDuration = 2.0;
    const double afterEnd = 2.0;
#endif

    double startTime = GetTime();
    double currentTime = 0;

    int imageWidth = splashBackgroundTexture.width;
    int imageHeight = splashBackgroundTexture.height;

    float scaleX = (float)baseScreenWidth / imageWidth;
    float scaleY = (float)baseScreenHeight / imageHeight;

    BeginDrawing();
    BeginMode2D(*camera);
    ClearBackground(RAYWHITE);
    EndMode2D();
    EndDrawing();

    while (currentTime < beforeStart)
    {
        WindowUpdate(camera);
        currentTime = GetTime() - startTime;

        BeginDrawing();
        BeginMode2D(*camera);
        float alpha = (float)(255.0 * (1.0 - fmin(currentTime / beforeStart, 1.0)));

        DrawTextureEx(splashBackgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);
        DrawRectangle(baseX, baseY, baseScreenWidth, baseScreenHeight, (Color) { 255, 255, 255, alpha });
        EndMode2D();
        EndDrawing();
    }

#if !DEBUG_FASTLOAD
    const Sound systemLoad = LoadSound(ASSETS_PATH"audio/Meow1.mp3");
    PlaySound(systemLoad);
#endif

    LoadGlobalAssets();

    // Reset time
    startTime = GetTime();

    while (currentTime < splashDuration)
    {
        WindowUpdate(camera);
        currentTime = GetTime() - startTime;

        int alpha = 0;

        // Calculate alpha based on the current time
        if (currentTime < fadeInDuration) {
            // Fading in
            alpha = (int)(255.0 * (currentTime / fadeInDuration));
        }
        else if (currentTime < fadeInDuration + stayDuration) {
            // Fully visible (staying)
            alpha = 255;
        }
        else {
            // Fading out
            alpha = (int)(255.0 * (1.0 - fmin((currentTime - fadeInDuration - stayDuration) / fadeOutDuration, 1.0)));
        }

        BeginDrawing();
        BeginMode2D(*camera);
        ClearBackground(RAYWHITE);
        DrawTextureEx(splashBackgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);
        DrawTextureEx(splashOverlayTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), (Color) { 255, 255, 255, alpha });
        
        EndMode2D();
        EndDrawing();
    }

    

    // Reset time
    startTime = GetTime();


    while (currentTime < afterEnd)
    {
        WindowUpdate(camera);
        currentTime = GetTime() - startTime;

        BeginDrawing();
        BeginMode2D(*camera);
        ClearBackground(RAYWHITE);
        DrawTextureEx(splashBackgroundTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);
        DrawTextureEx(splashOverlayTexture, (Vector2) { baseX, baseY }, 0.0f, fmax(scaleX, scaleY), WHITE);
        EndMode2D();
        EndDrawing();
    }

    // UnloadSound(systemLoad);

    MainMenuUpdate(camera, true);
}

int main()
{
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetTextureFilter(meowFont.texture, TEXTURE_FILTER_TRILINEAR);

    InitWindow(baseScreenWidth, baseScreenHeight, "SuperMeowMeow");
    InitAudioDevice();
    // Center of screen
    SetWindowPosition(200, 200);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.offset = (Vector2){ baseScreenWidth / 2.0f, baseScreenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    GameOptions _options;

    _options.difficulty = MEDIUM;
    _options.resolution = (Resolution){ 1280, 720 };
    _options.targetFps = 240;
    _options.fullscreen = false;  // Default to windowed mode

    options = &_options;


    SetTargetFPS(options->targetFps);
    SetRuntimeResolution(&camera, options->resolution.x, options->resolution.y);

    logoTexture = LoadTexture(ASSETS_PATH"image/elements/studio_logo.png");
    splashBackgroundTexture = LoadTexture(ASSETS_PATH"image/backgrounds/splash.png");
    splashOverlayTexture = LoadTexture(ASSETS_PATH"image/backgrounds/splash_overlay.png");

    SplashUpdate(&camera);
    return 0;
}
