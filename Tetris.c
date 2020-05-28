/*テトリスゲーム*/

#include "gba1.h"
/*ゲーム用の定義*/
#define LCD_GAME_WIDTH 10   //テトリス画面の横幅
#define LCD_GAME_HEIGHT 20  //テトリス画面の縦幅
#define ENDLINE 45          //ゲームの終了ライン
#define T_BLOCK 5           //ブロックの幅
#define BOTTOM 1            //ブロックの底
#define RIGHT 2             //ブロックの右側
#define LEFT 3              //ブロックの左側
/*色の定義*/
#define BLUE    BGR(0x00,0x00,0x1F)
#define YELLOW  BGR(0x1F,0x1F,0x00)
#define GREEN   BGR(0x00,0x1F,0x00)
#define RED     BGR(0x1F,0x00,0x00)
#define WHITE   BGR(0x1F,0x1F,0x1F)
#define CYAN		BGR(0x00,0x1F,0x1F)
#define BLACK   BGR(0x00,0x00,0x00)
#define MAZENTA	BGR(0x1F,0x00,0x1F)

/*グローバル変数宣言*/
hword gamePoint = 0;//自分の得点
hword blockType = 1;//ブロックのタイプ
volatile unsigned short time;//時間を格納する
hword blockList[10] = {//ゲームでブロックが出てくる順番
  0,3,2,1,3,1,0,2,1,3,2
};
hword colorList[4] = {//ブロック0,1,2,3に対応するブロックの色
  YELLOW,BLUE,RED,GREEN
};

/*描画用関数*/
void drawSquare(hword,hword,hword,hword,hword);
void paintBlock0(hword,point,point,hword);
void paintBlock1(hword,point,point,hword);
void paintBlock2(hword,point,point,hword);
void paintBlock3(hword,point,point,hword);
void judgeDeleteBlock(hword,point,point,hword,hword);
void selectedBlock(hword,hword,point,point,hword);
void rowDelete();

/*ゲーム画面描画関数*/
void gameExplain();
void startScreen();
void playScreen();
void endScreen();

/*ゲーム用関数*/
void next();
void display_time(hword);
hword color_check(hword,hword);
hword hitCheck(hword,point,point,hword);
hword block0HitCheck(hword,point,point,hword);
hword block1HitCheck(hword,point,point,hword);
hword block2HitCheck(hword,point,point,hword);
hword block3HitCheck(hword,point,point,hword);



/**************************************************/
/*********************main************************/
/*************************************************/
int main(void) {

  *((hword *)IOBASE) = 0xF03;

  gameExplain();
  startScreen();
  playScreen();
  endScreen();

  while(1);
  return;
}


/**************************************************/
/******************functions**********************/
/*************************************************/

/*四角形を描画する関数*/
void drawSquare(hword startX, hword endX, hword startY, hword endY, hword color) {
  hword i, j;
  for(i = startY; i < endY; i++) {
    for(j = startX; j < endX; j++) {
      draw_point(j, i, color);
    }
  }
}

/*STARTキーが押されるまで、無限ループする関数*/
void next() {
  hword key_in;
  while(1) {
      key_in = ~(*(hword *) KEY_STATUS) & KEY_ALL;
      if(key_in == KEY_START) {
        break;
      }
  }
}

/*ゲーム説明関数*/
void gameExplain() {
  locate(5,8);   prints("This is Tetoris game.");
  locate(3,16);  prints("The operation explanation");
  locate(3,17);  prints("is on the next screen.");
  locate(3,18);  prints("Please Press ENTAR key");
  next();

  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);
  locate(3,3);   prints("There are 4 Blocks");
  locate(3,5);   prints("The block will rotate");
  locate(3,6);   prints("90 degrees right");
  locate(3,7);   prints("when you press the");
  locate(3,9);   prints("ENTER key.");
  locate(3,11);  prints("The block move to the");
  locate(3,12);  prints("pressed arrow key direction.");
  locate(3,18);  prints("Please Press ENTAR key");
  next();

  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);
  locate(3,3);   prints("When the blocks aligned");
  locate(3,4);   prints("in horizontal row,");
  locate(3,5);   prints("blocks in that row vanish");
  locate(3,6);   prints("and points are added.");
  locate(3,18);  prints("Please Press ENTAR key");
  next();

  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);
  locate(3,3);   prints("If the block exceeds");
  locate(3,4);   prints("the REDLine of screen,");
  locate(3,5);   prints("the game is over");
  locate(3,14);  prints("Well then, Let's Enjoy!!!");
  locate(3,18);  prints("Please Press ENTAR key");
  next();
}

/*初期画面を描画する関数*/
void startScreen() {
  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);
  locate(9,7);  prints("TETRIS GAME");
  locate(3,18); prints("Please Press ENTAR key");
  next();
}

/*ゲーム処理を行う関数*/
void playScreen() {
  point start,end;//現在のブロックの位置
  point nbStart, nbEnd;//次のブロックの位置
  hword nextBlock,nextBlockType;
  hword key_in;
  hword push;
  hword beforeTime,drop,dropCount;
  hword gameEnd;
  nbStart.x = 105;nbStart.y = 45;
  nbEnd.x = 110;nbEnd.y = 50;
  beforeTime = 0;
  dropCount = 0;
  drop = 1;
  gameEnd = 0;

  locate(15, 0);
  prints("Time = ");
  /* �^�C�}�J�E���^�ݒ背�W�X�^ */
  *((unsigned short *)0x04000100) = 0xFF00;	// �^�C�}0 �J�E���^�ݒ�(���N���b�N1/2^24�b ��60n�b)
  *((unsigned short *)0x04000104) = 0xFF00;	// �^�C�}1 �J�E���^�ݒ�(���N���b�N1/2^16�b ��15.4u�b)
  *((unsigned short *)0x04000108) = 0xFF00;	// �^�C�}2 �J�E���^�ݒ�(���N���b�N1/2^8�b  ��3.93m�b)
  *((unsigned short *)0x0400010C) = 0x0000;	// �^�C�}3 �J�E���^�ݒ�(���N���b�N1�b)

  /* �^�C�}���䃌�W�X�^ */
  *((unsigned short *)0x04000102) = 0x0080;	// �^�C�}0 �����ݒ��i�^�C�}ON�C������OFF�C�J�X�P�[�hOFF�C�v���X�P�[���Ȃ��j
  *((unsigned short *)0x04000106) = 0x0084;	// �^�C�}1 �����ݒ��i�^�C�}ON�C������OFF�C�J�X�P�[�hON�C�v���X�P�[���Ȃ��j
  *((unsigned short *)0x0400010A) = 0x0084;	// �^�C�}2 �����ݒ��i�^�C�}ON�C������OFF�C�J�X�P�[�hON�C�v���X�P�[���Ȃ��j
  *((unsigned short *)0x0400010E) = 0x0084;	// �^�C�}3 �����ݒ��i�^�C�}ON�C������OFF�C�J�X�P�[�hON�C�v���X�P�[���Ȃ��j

  /*ステージ描画*/
  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);//スクリーン全体の色
  drawSquare(5, LCD_GAME_WIDTH*T_BLOCK+35, 40, 45, RED);//ゲームオーバーのライン
  drawSquare(88, 138, 24, 64, WHITE);//次のブロックを描画するエリアの外枠
  drawSquare(15, LCD_GAME_WIDTH*T_BLOCK+25, 5, LCD_GAME_HEIGHT*T_BLOCK+50, WHITE);//ゲームの外枠
  drawSquare(20, LCD_GAME_WIDTH*T_BLOCK+20, 10, LCD_GAME_HEIGHT*T_BLOCK+45, BLACK);//ゲームの内枠

  /*ゲーム処理*/
  while(!gameEnd) {
    start.x = 50;start.y = 20;//初期化
    end.x = 55;end.y = 25;//初期化
    push = 0;//初期化
    dropCount++;//ブロックが着地した回数
    if(dropCount > 9) dropCount = 0;//初期化
    nextBlock = mod(dropCount+1,10);
    blockType = blockList[dropCount];//blockのリストから、次のブロックを決定している
    nextBlockType = blockList[nextBlock];//次のブロック
    drawSquare(93, 133, 29, 59,BLACK);//次のブロックを描画するエリア
    selectedBlock(nextBlockType, 0, nbStart, nbEnd, colorList[nextBlockType]);//次のブロックの表示

    /*ブロックが着地するまでの処理*/
    while(1) {
      /*時間計測と1秒ごとのブロック落下*/
      time = *((unsigned short *)0x0400010C);
      drop = (time - beforeTime) > 0 ? 1 : 0;
      if(drop == 1) {
        judgeDeleteBlock(push,start,end,BLACK,0);
        start.y += 5;
        end.y += 5;
      }
      beforeTime = time;

      /*案内板*/
      locate(11,2);   prints("NEXT");
      locate(18,3);   prints("Time = ");
      locate(25,3);   display_time(time);
      locate(18,5);   prints("point = ");
      locate(26,5);   printn(gamePoint);
      locate(18,7);   prints("move DOWN:");
      locate(18,8);   prints("Down Key");
      locate(18,10);  prints("move LEFT:");
      locate(18,11);  prints("Left Key");
      locate(18,13);  prints("move RIGHT:");
      locate(18,14);  prints("Right Key");
      locate(18,16);  prints("Rocate 90:");
      locate(18,17);  prints("Enter Key");

      /*回転と移動*/
      key_in = ~(*(hword *) KEY_STATUS) & KEY_ALL;
      switch(key_in) {
        case KEY_START:
          judgeDeleteBlock(push,start,end,BLACK,0);
          push++;
          if(push > 3) push = 0;
          judgeDeleteBlock(push,start,end,colorList[blockType],1);
          break;
        case KEY_LEFT:
          if(!hitCheck(LEFT, start, end, push)) {
            judgeDeleteBlock(push,start,end,BLACK,0);
            start.x -= 5;
            end.x -= 5;
          }
          break;
        case KEY_RIGHT:
          if(!hitCheck(RIGHT, start, end, push)) {
            judgeDeleteBlock(push,start,end,BLACK,0);
            start.x += 5;
            end.x += 5;
          }
          break;
        case KEY_DOWN:
          if(!hitCheck(BOTTOM, start, end, push)) {
            judgeDeleteBlock(push,start,end,BLACK,0);
            start.y += 5;
            end.y += 5;
            }
            break;
      }
      judgeDeleteBlock(push,start,end,colorList[blockType],1);

      /*矢印キー押し続けても変更は一度だけ起こるように設定*/
      while(1) {
          key_in = ~(*(hword *) KEY_STATUS) & (KEY_ALL-KEY_A);
          if(key_in == 0x000) break;
      }

      /*何かしらに着地した際の処理*/
      if(hitCheck(BOTTOM, start, end, push)) {
        /*1~2秒立つまでは左右移動を可能に*/
        while((time - beforeTime) < 1) {
          time = *((unsigned short *)0x0400010C);
          switch(key_in) {
            case KEY_LEFT:
              if(!hitCheck(LEFT, start, end, push)) {
                judgeDeleteBlock(push,start,end,BLACK,0);
                start.x -= 5;
                end.x -= 5;
              }
              break;
            case KEY_RIGHT:
              if(!hitCheck(RIGHT, start, end, push)) {
                judgeDeleteBlock(push,start,end,BLACK,0);
                start.x += 5;
                end.x += 5;
              }
              break;
            }
            judgeDeleteBlock(push,start,end,colorList[blockType],1);
        }
        /*横列削除判定とゲーム終了判定*/
        rowDelete();
        if(start.y <= ENDLINE) gameEnd = 1;
        break;
      }
    }
  }
}

/*ブロックを消すか描画する関数*/
void judgeDeleteBlock(hword push, point start, point end, hword color, hword isNodelete) {
  if(isNodelete) selectedBlock(blockType, push, start, end, color);
  else           selectedBlock(blockType, push, start, end, BLACK);

}

/*指定したブロックを描画する関数*/
void selectedBlock(hword type, hword push, point start, point end, hword color) {
  switch(type) {
    case 0: paintBlock0(push, start, end, color); break;
    case 1: paintBlock1(push, start, end, color); break;
    case 2: paintBlock2(push, start, end, color); break;
    case 3: paintBlock3(push, start, end, color); break;
  }
}
/*
  ****
  ****
****
****
の形を描画する関数
*/
void paintBlock0(hword push, point start, point end, hword color) {
  switch(mod(push,2)) {
    case 0:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(end.x, end.x+T_BLOCK, start.y, end.y, color);
      drawSquare(end.x, end.x+T_BLOCK, start.y-T_BLOCK, start.y, color);
      drawSquare(end.x+T_BLOCK, end.x+2*T_BLOCK, start.y-T_BLOCK, start.y, color);
      break;
    case 1:
      drawSquare(start.x, end.x, start.y-2*T_BLOCK, end.y-2*T_BLOCK, color);
      drawSquare(start.x, end.x, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y, end.y, color);
      break;
  }
}

/*
****
****
****
****
の形を描画する関数
*/
void paintBlock1(hword push, point start, point end, hword color) {
  drawSquare(start.x, end.x+T_BLOCK, start.y-T_BLOCK, end.y, color);
}

/*
********
********
の形を描画する関数
*/
void paintBlock2(hword push, point start, point end, hword color) {
  switch(mod(push,2)) {
    case 0:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y, end.y, color);
      drawSquare(start.x+2*T_BLOCK, end.x+2*T_BLOCK, start.y, end.y, color);
      drawSquare(start.x+3*T_BLOCK, end.x+3*T_BLOCK, start.y, end.y, color);
      break;
    case 1:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(start.x, end.x, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x, end.x, start.y-2*T_BLOCK, end.y-2*T_BLOCK, color);
      drawSquare(start.x, end.x, start.y-3*T_BLOCK, end.y-3*T_BLOCK, color);
      break;
  }
}

/*
    **
    **
******
******
の形を描画する関数
*/
void paintBlock3(hword push, point start, point end, hword color) {
  switch(push) {
    case 0:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y, end.y, color);
      drawSquare(start.x+2*T_BLOCK, end.x+2*T_BLOCK, start.y, end.y, color);
      drawSquare(start.x+2*T_BLOCK, end.x+2*T_BLOCK, start.y-T_BLOCK, end.y-T_BLOCK, color);
      break;
    case 1:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y, end.y, color);
      drawSquare(start.x, end.x, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x, end.x, start.y-2*T_BLOCK, end.y-2*T_BLOCK, color);
      break;
    case 2:
      drawSquare(start.x, end.x, start.y, end.y, color);
      drawSquare(start.x, end.x, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x+2*T_BLOCK, end.x+2*T_BLOCK, start.y-T_BLOCK, end.y-T_BLOCK, color);
      break;
    case 3:
      drawSquare(start.x, end.x, start.y-2*T_BLOCK, end.y-2*T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y-2*T_BLOCK, end.y-2*T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y-T_BLOCK, end.y-T_BLOCK, color);
      drawSquare(start.x+T_BLOCK, end.x+T_BLOCK, start.y, end.y, color);
      break;
  }
}

/*全ブロックの判定を行う関数*/
hword hitCheck(hword direction, point start, point end, hword push) {
  switch(blockType) {
    case 0: block0HitCheck(direction, start, end, push);  break;
    case 1: block1HitCheck(direction, start, end, push);  break;
    case 2: block2HitCheck(direction, start, end, push);  break;
    case 3: block3HitCheck(direction, start, end, push);  break;
  }
}

/*ブロック0の判定を行う関数*/
hword block0HitCheck(hword direction, point start, point end, hword push) {
  if(direction == BOTTOM) {//底の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(start.x+2, end.y+1) != BLACK)          return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)            return 1;
        if(color_check(end.x+T_BLOCK+2, start.y+1) != BLACK)  return 1;
        return 0;
        break;
      case 1:
        if(color_check(start.x+2, start.y+1) != BLACK)         return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)             return 1;
        return 0;
        break;
    }
  }else if(direction == RIGHT) {//右の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(end.x+2*T_BLOCK+1, start.y-2) != BLACK) return 1;
        if(color_check(end.x+T_BLOCK+1, start.y+2) != BLACK)   return 1;
        return 0;
        break;
      case 1:
        if(color_check(end.x+T_BLOCK+1, start.y-2) != BLACK)   return 1;
        if(color_check(end.x+T_BLOCK+1, start.y+2) != BLACK)   return 1;
        return 0;
        break;
    }
  }else if(direction == LEFT) {//左の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(start.x-1, start.y+2) != BLACK)         return 1;
        if(color_check(end.x-1, start.y-2) != BLACK)           return 1;
        return 0;
        break;
      case 1:
        if(color_check(end.x-1, start.y+2) != BLACK)           return 1;
        if(color_check(start.x-1, start.y-2) != BLACK)         return 1;
        if(color_check(start.x-1, start.y-T_BLOCK-2) != BLACK) return 1;
        return 0;
        break;
    }
  }
}

/*ブロック1の判定を行う関数*/
hword block1HitCheck(hword direction, point start, point end, hword push) {
  if(direction == BOTTOM) {//底の当たり判定
      if(color_check(end.x-2, end.y+1) != BLACK)          return 1;
      if(color_check(end.x+2, end.y+1) != BLACK)          return 1;
      return 0;
  }else if(direction == RIGHT) {//右の当たり判定
      if(color_check(end.x+T_BLOCK+1, start.y-2) != BLACK) return 1;
      if(color_check(end.x+T_BLOCK+1, start.y+2) != BLACK) return 1;
      return 0;
  }else if(direction == LEFT) {//左の当たり判定
      if(color_check(start.x-1, start.y+2) != BLACK)       return 1;
      if(color_check(start.x-1, start.y-2) != BLACK)       return 1;
      return 0;
  }
}

/*ブロック2の判定を行う関数*/
hword block2HitCheck(hword direction, point start, point end, hword push) {
  if(direction == BOTTOM) {//底の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(start.x+2, end.y+1) != BLACK)          return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)            return 1;
        if(color_check(end.x+T_BLOCK+2, end.y+1) != BLACK)    return 1;
        if(color_check(end.x+2*T_BLOCK+2, end.y+1) != BLACK)  return 1;
        return 0;
        break;
      case 1:
        if(color_check(start.x+2, end.y+1) != BLACK)          return 1;
        return 0;
        break;
    }
  }else if(direction == RIGHT) {//右の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(end.x+3*T_BLOCK+1, end.y-2) != BLACK)  return 1;
        return 0;
        break;
      case 1:
        if(color_check(end.x+1, end.y-2) != BLACK)            return 1;
        if(color_check(end.x+1, end.y-T_BLOCK-2) != BLACK)    return 1;
        if(color_check(end.x+1, end.y-2*T_BLOCK-2) != BLACK)  return 1;
        if(color_check(end.x+1, end.y-3*T_BLOCK-2) != BLACK)  return 1;
        return 0;
        break;
    }
  }else if(direction == LEFT) {//左の当たり判定
    switch(mod(push,2)) {
      case 0:
        if(color_check(start.x-1, end.y-2) != BLACK)          return 1;
        return 0;
        break;
      case 1:
        if(color_check(start.x-1, end.y-2) != BLACK)            return 1;
        if(color_check(start.x-1, end.y-T_BLOCK-2) != BLACK)    return 1;
        if(color_check(start.x-1, end.y-2*T_BLOCK-2) != BLACK)  return 1;
        if(color_check(start.x-1, end.y-3*T_BLOCK-2) != BLACK)  return 1;
        return 0;
        break;
    }
  }
}

/*ブロック3の判定を行う関数*/
hword block3HitCheck(hword direction, point start, point end, hword push) {
  if(direction == BOTTOM) {//底の当たり判定
    switch(push) {
      case 0:
        if(color_check(start.x+2, end.y+1) != BLACK)            return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)              return 1;
        if(color_check(end.x+T_BLOCK+2, end.y+1) != BLACK)      return 1;
        return 0;
        break;
      case 1:
        if(color_check(start.x+2, end.y+1) != BLACK)            return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)              return 1;
        return 0;
        break;
      case 2:
        if(color_check(start.x+2, end.y+1) != BLACK)            return 1;
        if(color_check(end.x+2, start.y+1) != BLACK)            return 1;
        if(color_check(end.x+T_BLOCK+2, start.y+1) != BLACK)    return 1;
        return 0;
        break;
      case 3:
        if(color_check(start.x+2, start.y-T_BLOCK+1) != BLACK)  return 1;
        if(color_check(end.x+2, end.y+1) != BLACK)              return 1;
        return 0;
        break;
    }
  }else if(direction == RIGHT) {//右の当たり判定
    switch(push) {
      case 0:
        if(color_check(end.x+2*T_BLOCK+1, start.y-2) != BLACK)          return 1;
        if(color_check(end.x+2*T_BLOCK+1, start.y+2) != BLACK)            return 1;
        return 0;
        break;
      case 1:
        if(color_check(end.x+1, start.y-2) != BLACK)                    return 1;
        if(color_check(end.x+1, start.y-T_BLOCK-2) != BLACK)            return 1;
        if(color_check(end.x+T_BLOCK+1, start.y+2) != BLACK)            return 1;
        return 0;
        break;
      case 2:
        if(color_check(end.x+2*T_BLOCK+1, start.y-2) != BLACK)          return 1;
        if(color_check(end.x+1, start.y+2) != BLACK)                    return 1;
        return 0;
        break;
      case 3:
        if(color_check(end.x+T_BLOCK+1, start.y-2) != BLACK)            return 1;
        if(color_check(end.x+T_BLOCK+1, start.y-T_BLOCK-2) != BLACK)    return 1;
        if(color_check(end.x+T_BLOCK+1, start.y+2) != BLACK)            return 1;
        return 0;
        break;
    }
  }else if(direction == LEFT) {//左の当たり判定
    switch(push) {
      case 0:
        if(color_check(start.x-1, start.y+2) != BLACK)                  return 1;
        if(color_check(start.x+2*T_BLOCK-1, start.y-2) != BLACK)        return 1;
        return 0;
        break;
      case 1:
        if(color_check(start.x-1, start.y-2) != BLACK)                  return 1;
        if(color_check(start.x-1, start.y-T_BLOCK-2) != BLACK)          return 1;
        if(color_check(start.x-1, start.y+2) != BLACK)                  return 1;
        return 0;
        break;
      case 2:
        if(color_check(start.x-1, start.y-2) != BLACK)                  return 1;
        if(color_check(start.x-1, start.y+2) != BLACK)                  return 1;
        return 0;
        break;
      case 3:
        if(color_check(start.x-1, start.y-T_BLOCK-2) != BLACK)          return 1;
        if(color_check(end.x-1, start.y-2) != BLACK)                    return 1;
        if(color_check(end.x-1, start.y+2) != BLACK)                    return 1;
        return 0;
        break;
    }
  }
}

/*列の削除処理と、得点カウント*/
void rowDelete() {
  hword i, j, k;
  hword fullRow;
  hword bonus;
  bonus = 0;
  fullRow = 0;

  for(i = 0; i < LCD_GAME_HEIGHT; i++) {//全横列
    /*横一列検査*/
    for(j = LCD_GAME_HEIGHT*T_BLOCK+40-i*T_BLOCK; j < LCD_GAME_HEIGHT*T_BLOCK+45-i*T_BLOCK; j++) {
      for(k = 20; k < LCD_GAME_WIDTH*T_BLOCK+20; k++) {
        if(color_check(k, j) == BLACK) {//一列の中に一個でも黒が混じっていれば消せない
          fullRow = 0;
          break;
        }
        fullRow = 1;//横一列埋まってる
      }
    }

    /*横一列消す,そして加点*/
    if(fullRow) {
      drawSquare(20,
        LCD_GAME_WIDTH*T_BLOCK+20,
        LCD_GAME_HEIGHT*T_BLOCK+40-i*T_BLOCK,
        LCD_GAME_HEIGHT*T_BLOCK+45-i*T_BLOCK,
        BLACK);
        bonus ++;//一度に消えた列が多いほどボーナスがつく
        gamePoint += 10*bonus;
        fullRow = 0;
    }
  }
}

/*終了画面を描画する関数*/
void endScreen() {
  drawSquare(0, LCD_WIDTH, 0, LCD_HEIGHT, BLACK);
  locate(7, 7);   prints("Your Point is");
  locate(5, 10);  printn(gamePoint);prints("points!!!!!!");
}

/*時間を計測する関数*/
void display_time(hword val) {
	byte char_data[] = "0123456789";
	byte buf[6];
	hword tmp;
	int i;
	i = 3;
	buf[i+1] = 0;
	for(; i >= 0; i--) {
		buf[i] = char_data[mod(val, 10)];
		val = div(val, 10);
	}
	prints(buf);
	return;
}

/*指定の位置の色を判定する関数*/
hword color_check(hword x, hword y){
	hword *ptr;
	ptr = (hword*)VRAM + x + (y * 240);
	hword color = *ptr;
	return color;
}
