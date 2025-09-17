#include "SameGame.h"

//
// TODOs:
// Choose color palettes?
// Consider alternatives to TextScroll for displaying scoring
// Speedrun mode

void SameGame::Setup() {
  ResetGame(true);
}

void SameGame::Loop()
{
  struct KeyEvent keyEvent;

  if(renderTimer.Tick(1000/Device::LED::fps))
  {
    while (MatrixOS::KEYPAD::Get(&keyEvent))
    { KeyEventHandler(keyEvent.id, &keyEvent.info); }
    Render();
  }
}

SameGame::boardInfo SameGame::Rpc(Point pos)
{
  boardInfo ans;
  ans.color = 0;
  ans.floodmark = 0;
  if(pos.x < 0 || pos.x >= 8 || pos.y < 0 || pos.y >= 8) // off board
  {
    return ans;
  }
  ans = board[pos.y][pos.x];
  return ans;
}

void SameGame::Wpc(Point pos, boardInfo cell)
{
  if(pos.x < 0 || pos.x >= 8 || pos.y < 0 || pos.y >= 8) // off board
  {
    return;
  }
  board[pos.y][pos.x] = cell;
}

bool SameGame::Fall(Point pos, bool create)
{
  // Make things from above fall into the cell
  boardInfo v0 = Rpc(pos);
  if (v0.color != 0)
    return false; // we can't fall anything into it

  Point above;
  boardInfo v1;
  v1.color = 0;
  v1.floodmark = 0;
  above.x = pos.x;
  above.y = pos.y - 1;
  if (create) // we are filling the cell as soon as it's empty
  {
    v1.color = GetRandomColor();
    board[pos.y][pos.x] = v1;
    return true;
  }
  else
  {    
    v1 = Rpc(above);
    if (v1.color != 0) // we have a thing to fall
    {
      Wpc(pos, v1); // move it down
      v1.color = 0;
      v1.floodmark = 0;
      Wpc(above, v1); // then erase the old pos
      return true;
    }
    else { // nothing fell this turn
      return false;
    }
  }
}

bool SameGame::MarkCell(Point posadj, boardInfo cur)
{
    boardInfo adj;
    adj = Rpc(posadj);
    if (adj.floodmark == 0 && adj.color == cur.color && cur.floodmark == 1)
    {
      adj.floodmark = 1;
      Wpc(posadj, adj);
      return true;
    }
    return false;
}

bool SameGame::MarkAdjacent(Point pos)
{
  boardInfo cur;
  Point posadj;
  cur = Rpc(pos);
  if (cur.color != 0 && cur.floodmark == 1) { // only allow if non-empty and marked
    bool ans;
    ans = false;

    // left
    posadj = pos;
    posadj.x -= 1;
    ans = ans | MarkCell(posadj, cur);

    // right
    posadj = pos;
    posadj.x += 1;
    ans = ans | MarkCell(posadj, cur);

    // above
    posadj = pos;
    posadj.y -= 1;
    ans = ans | MarkCell(posadj, cur);

    // below
    posadj = pos;
    posadj.y += 1;
    ans = ans | MarkCell(posadj, cur);

    return ans;
  }
  return false;
}

uint8_t SameGame::CellFlood(Point pos, uint8_t max_iterations)
{
  // mark the initial point and clear the rest
  for(uint8_t y = 0; y < 8; y++)
  {
    for(uint8_t x = 0; x < 8; x++)
    {
        board[y][x].floodmark = 0;
    }
  }
  board[pos.y][pos.x].floodmark = 1;
  uint8_t ans = 0;
  if (MarkAdjacent(pos)) {
    // start a loop after marking the initial point...
    bool cont;
    while (cont && max_iterations > 0) {
      max_iterations--;
      ans = 0; 
      cont = false;
      // iterate over the entire board...
      for(uint8_t y = 0; y < 8; y++)
      {
        for(uint8_t x = 0; x < 8; x++)
        {
            if (board[y][x].floodmark == 1) { // for all marked points...
              ans++; // add to the count 
              if (MarkAdjacent(Point(x,y))) {
                cont = true; // continue if we've found more markers.
              }
            }
        }
      }
    }
  }
  return ans;
}

bool SameGame::CompactColumn(uint8_t x)
{
  // scan for column emptiness
  for(uint8_t y = 0; y < 8; y++)
  {
    if(board[y][x].color != 0) 
    {
      return false;
    }
  }
  if (x+1 >= 8) { // we can't swap above this column
    return false;
  }
  // execute swap
  bool swapped = false;
  for(uint8_t y = 0; y < 8; y++)
  {
    if(board[y][x+1].color != 0) 
    {
      swapped = true;
      board[y][x].color = board[y][x+1].color;
      board[y][x+1].color = 0;
    }
  }
  return swapped;  
}

void SameGame::Place(Point pos)
{

  boardInfo cur;
  cur = Rpc(pos);

  if(gameState != Waiting) // not time to take input
  {
    return;
  }
  if(cur.color == 0) // empty
  {
    return;
  }

  // apply CellFlood to discover what's marked and how many

  uint8_t cleared = CellFlood(pos, 255); // theoretical max would be a 36-length space-filling curve, I think

  // scan and eliminate all marked

  Point scanpos;
  if (cleared > 1) {
    for(uint8_t y = 0; y < 8; y++)
    {
      for(uint8_t x = 0; x < 8; x++)
      {
          scanpos.x = x;
          scanpos.y = y;
          cur = Rpc(scanpos);
          if (cur.floodmark == 1)
          {
            lastColor = cur.color;
            cur.floodmark = 0;
            cur.color = 0;
            Wpc(scanpos, cur);
            cleared++;
          }
      }
    }

    // perform falling

    //  Use n(n-2) scoring:
    //  0, 3, 8, 15, 24, 35, 48... (256*254)=65536-512
    clearedScore = cleared * (cleared - 2);
    score += clearedScore;
    string score_str = std::to_string(clearedScore);
    MatrixOS::UIUtility::TextScroll(score_str, GetCellColor(lastColor), 20, false);

    gameState = Moving;
    MLOGD("SameGame", "Moving");
    lastEventTime = MatrixOS::SYS::Millis();
  }

}

uint8_t SameGame::GetRandomNumber(uint8_t upperbound, uint8_t lowerbound) {
  return rand() % (upperbound - lowerbound + 1) + lowerbound;
}
uint8_t SameGame::GetRandomColor() {
  return GetRandomNumber(numColors.Get(), 1);
}

void SameGame::RenderXferBoard()
{
    for(uint8_t y = 0; y < 8; y++)
    {
      for(uint8_t x = 0; x < 8; x++)
      {
          MatrixOS::LED::SetColor(Point(x, y), GetCellColor(board[y][x].color));
      }
    }
}

void SameGame::Render()
{
  uint32_t timeSinceEvent = MatrixOS::SYS::Millis() - lastEventTime;
  if(gameState == SetupBoard)
  {

    if (timeSinceEvent >= 100){ // animation rate
      bool fell = false;
      for(uint8_t y = 0; y < 8; y++)
      {
        bool docreate = (y == 7); // create new rows at y = 0
        for(uint8_t x = 0; x < 8; x++)
        {
            fell = fell | Fall(Point(x,7-y), docreate); // fall bottom-upwards
        }
      }
      RenderXferBoard();
      if (fell) // reset the event timer and continue iterating
      {
          lastEventTime = MatrixOS::SYS::Millis();
      }
      if(!fell && timeSinceEvent >= 250) // pause after fall completes
      {
        gameState = Waiting;
        RenderXferBoard();
        MLOGD("SameGame", "Waiting");
        lastEventTime = MatrixOS::SYS::Millis();
      }
    }
  }
  else if(gameState == Waiting)
  {

    MatrixOS::LED::FillPartition("Underglow", ColorEffects::ColorBreath(GetCellColor(lastColor), 
        2000, lastEventTime - 500));

  }
  else if(gameState == Moving)
  {

    MatrixOS::LED::FillPartition("Underglow", ColorEffects::ColorSaw(GetCellColor(lastColor), 
        250, lastEventTime - 500));

    if (timeSinceEvent >= 100){ // animation rate
      bool fell = false;
      for(uint8_t y = 0; y < 8; y++)
      {
        for(uint8_t x = 0; x < 8; x++)
        {
            fell = fell | Fall(Point(x,7-y), false); // fall bottom-upwards
        }
      }
      RenderXferBoard();

      if (fell) // reset the event timer and continue iterating
      {
          lastEventTime = MatrixOS::SYS::Millis();
      }
      else // continue to compaction
      {

        gameState = Compacting;
        MLOGD("SameGame", "Compacting");
        lastEventTime = MatrixOS::SYS::Millis();
      }
    }
  }
  else if(gameState == Compacting)
  {
    if (timeSinceEvent >= 100){ // animation rate
      bool compact = false;
      for(uint8_t x = 0; x < 8; x++)
      {
          compact = compact | CompactColumn(x);
      }
      RenderXferBoard();
      if (compact) // reset the event timer and continue iterating
      {
          lastEventTime = MatrixOS::SYS::Millis();
      }
      else // after compact completes
      {

        // detect the endgame by counting cells and 
        // doing a 1-iteration floodmark on every cell to find a 2+ available

        uint8_t best = 0;
        uint8_t cells_on_board = 0;
        for (uint8_t y=0; y<8; y++) 
        {
          for (uint8_t x=0; x<8; x++) 
          {
            if (board[y][x].color != 0)
            {
              cells_on_board++;
              uint8_t cleared = CellFlood(Point(x,y), 1);
              if(cleared > best)
              {
                best = cleared;
              }
            }
          }
        }

        if (cells_on_board == 0)
        {
          MatrixOS::UIUtility::TextScroll("Board cleared - 2x score!", Color(0x44FF44), 20, false);
          score = score * 2;
          string score_str = std::to_string(score) + " points - Game Over ";
          MatrixOS::UIUtility::TextScroll(score_str, Color(0xFFFFFF), 15, true);
          gameState = Ended;
          MLOGD("SameGame", "Ended");
          lastEventTime = MatrixOS::SYS::Millis();
        }
        else if (best < 2)
        {
          MatrixOS::UIUtility::TextScroll("No valid moves!", Color(0xFF44FF), 20, false);
          string score_str = std::to_string(score) + " points - Game Over ";
          MatrixOS::UIUtility::TextScroll(score_str, Color(0xFFFFFF), 15, true);
          gameState = Ended;
          MLOGD("SameGame", "Ended");
          lastEventTime = MatrixOS::SYS::Millis();
        }
        else {
          gameState = Waiting;
          RenderXferBoard();
          MLOGD("SameGame", "Waiting");
          lastEventTime = MatrixOS::SYS::Millis();
        }

      }
    }
  }
  else if(gameState == Ended)
  {
    RenderXferBoard();
    lastEventTime = MatrixOS::SYS::Millis();
    MatrixOS::LED::FillPartition("Underglow", ColorEffects::Rainbow(1000, 0));
  }

  MatrixOS::LED::Update();
}

bool SameGame::ConfirmMenu()
{
  bool confirmed = false;
  UI confirmUI("Reset Game?", Color(0xFF0000), false);

  UIButton cancelResetBtn;
  cancelResetBtn.SetName("Cancel");
  cancelResetBtn.SetColor(Color(0xFF0000));
  cancelResetBtn.SetSize(Dimension(2, 2));
  cancelResetBtn.OnPress([&]() -> void {
    confirmed = false;
    confirmUI.Exit();
  });
  confirmUI.AddUIComponent(cancelResetBtn, Point(1, 3));

  UIButton confirmResetBtn;
  confirmResetBtn.SetName("Confirm");
  confirmResetBtn.SetColor(Color(0x00FF00));
  confirmResetBtn.SetSize(Dimension(2, 2));
  confirmResetBtn.OnPress([&]() -> void {
    confirmed = true;
    confirmUI.Exit();
  });
  confirmUI.AddUIComponent(confirmResetBtn, Point(5, 3));

  confirmUI.Start();

  return confirmed;
}

bool SameGame::ResetGame(bool confirmed)
{
  srand(MatrixOS::SYS::Millis());
  if((!confirmed && !ConfirmMenu()))
  {
      return false;
  }
  
  for(uint8_t y = 0; y < 8; y++)
  {
    for(uint8_t x = 0; x < 8; x++)
    {
      board[y][x].color = 0;
      board[y][x].floodmark = 0;
    }
  }
  RenderXferBoard();

  if (numColors.Get() < 2 || numColors.Get() > 5) // enforce color count
    numColors.Set(4);
  lastColor = numColors.Get();

  gameState = SetupBoard;
  MLOGD("SameGame", "SetupBoard");

  lastEventTime = MatrixOS::SYS::Millis();

  return true;
}

Color SameGame::GetCellColor(uint8_t cell)
{
  if(cell == 0)
  {
    return Color(0x000000);
  }
  else if(cell == 1)
  {
    return Color(0xFF0000);
  }
  else if(cell == 2)
  {
    return Color(0x00FF00);
  }
  else if(cell == 3)
  {
    return Color(0x0000FF);
  }
  else if(cell == 4)
  {
    return Color(0xFFFF00);
  }
  else if(cell == 5)
  {
    return Color(0xFF00FF);
  }
  else {
    return Color(0xFFFFFF);
  }
}

void SameGame::KeyEventHandler(uint16_t keyID, KeyInfo* keyInfo) {
  if (keyID == FUNCTION_KEY)
  {
    if (keyInfo->state == PRESSED)
    {
      Settings();
    }
    return;
  }

  if(gameState == Waiting)
  {
    Point xy = MatrixOS::KEYPAD::ID2XY(keyID);
    
    if (xy && keyInfo->state == PRESSED)  // IF XY is valid, means it's on the main grid
    {
      Place(xy);
    }
  }
}


void SameGame::Settings() {
  UI settingsUI("Settings", Color(0x00FFFF), true);

  UIButton resetGameBtn;
  resetGameBtn.SetName("Reset Game");
  resetGameBtn.SetColorFunc([&]() -> Color { return Color(0xFF0000); });
  resetGameBtn.OnPress([&]() -> void {
    if(ResetGame(false)) { settingsUI.Exit(); }
  });
  resetGameBtn.SetSize(Dimension(1, 2));
  settingsUI.AddUIComponent(resetGameBtn, Point(7, 3));

  Color selectedColor = Color(0xFFFFFF);
  Color deselectedColor2 = Color(0x222222);
  Color deselectedColor3 = Color(0x662222);
  Color deselectedColor4 = Color(0xAA2222);
  Color deselectedColor5 = Color(0xEE2222);

  UIButton numColors2;
  numColors2.SetName("Two Colors");
  numColors2.SetColorFunc([&]() -> Color { return (numColors.Get() == 2) ? selectedColor : deselectedColor2; });
  numColors2.OnPress([&]() -> void { numColors.Set(2); });
  numColors2.SetSize(Dimension(1, 2));
  settingsUI.AddUIComponent(numColors2, Point(0, 0));

  UIButton numColors3;
  numColors3.SetName("Three Colors");
  numColors3.SetColorFunc([&]() -> Color { return (numColors.Get() == 3) ? selectedColor : deselectedColor3; });
  numColors3.OnPress([&]() -> void { numColors.Set(3); });
  numColors3.SetSize(Dimension(1, 3));
  settingsUI.AddUIComponent(numColors3, Point(1, 0));

  UIButton numColors4;
  numColors4.SetName("Four Colors");
  numColors4.SetColorFunc([&]() -> Color { return (numColors.Get() == 4) ? selectedColor : deselectedColor4; });
  numColors4.OnPress([&]() -> void { numColors.Set(4); });
  numColors4.SetSize(Dimension(1, 4));
  settingsUI.AddUIComponent(numColors4, Point(2, 0));

  UIButton numColors5;
  numColors5.SetName("Five Colors");
  numColors5.SetColorFunc([&]() -> Color { return (numColors.Get() == 5) ? selectedColor : deselectedColor5; });
  numColors5.OnPress([&]() -> void { numColors.Set(5); });
  numColors5.SetSize(Dimension(1, 5));
  settingsUI.AddUIComponent(numColors5, Point(3, 0));
  
  // Second, set the key event handler to match the intended behavior
  settingsUI.SetKeyEventHandler([&](KeyEvent* keyEvent) -> bool {
    // If function key is hold down. Exit the application
    if (keyEvent->id == FUNCTION_KEY)
    {
      if (keyEvent->info.state == HOLD)
      {
        Exit();  // Exit the application.
      }
      else if (keyEvent->info.state == RELEASED)
      {
        settingsUI.Exit();  // Exit the UI
      }

      return true;  // Block UI from to do anything with FN, basically this function control the life cycle of the UI
    }
    return false;  // Nothing happened. Let the UI handle the key event
  });
  
  // The UI object is now fully set up. Let the UI runtime to start and take over.
  settingsUI.Start();
}
