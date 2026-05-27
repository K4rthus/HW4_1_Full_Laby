#include "MazeGameMode.h"
#include "MazePlayerController.h"

AMazeGameMode::AMazeGameMode()
{
	PlayerControllerClass = AMazePlayerController::StaticClass();
}