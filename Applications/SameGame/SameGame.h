#pragma once

#include "MatrixOS.h"
#include "ui/UI.h"
#include "ui/UIUtilities.h"
#include "Application.h"


class SameGame : public Application {
 public:
  enum GameState {
    Waiting,
    NoValidMoves,
    Moving,
    Compacting,
    SetupBoard,
    Ended
  };

  struct boardInfo {
    uint8_t color : 5; // 0 = empty, 1 = Red, 2 = Green, 3 = Blue, 4 = Yellow, 5 = Purple
    uint8_t floodmark : 2; // 0 = unknown, 1 = marked
  };

  Timer renderTimer;

  boardInfo board[8][8];

  GameState gameState;

  uint32_t lastEventTime;
  uint32_t lastSubEventTime;

  uint32_t score;
  uint32_t clearedScore;
  uint8_t lastColor;

  inline static Application_Info info = {
      .name = "SameGame",
      .author = "Triplefox",
      .color = Color(0x008F00),
      .version = 1,
      .visibility = true,
  };
  void Setup() override;
  void Loop() override;
  void KeyEventHandler(uint16_t keyID, KeyInfo* keyInfo);
  void Place(Point pos);

  boardInfo Rpc(Point pos);
  void Wpc(Point pos, boardInfo cell);
  bool Fall(Point pos, bool create);
  bool MarkCell(Point pos, boardInfo cur);
  bool MarkAdjacent(Point pos);
  uint8_t CellFlood(Point pos, uint8_t max_iterations);
  bool CompactColumn(uint8_t x);

  void RenderXferBoard();
  void Render();

  void Settings();

  bool ConfirmMenu();
  bool ResetGame(bool confirmed);
  uint8_t GetRandomNumber(uint8_t upperbound, uint8_t lowerbound);
  uint8_t GetRandomColor();

  Color GetCellColor(uint8_t cell);

  CreateSavedVar("SameGame", numColors, uint8_t, 4);

};



