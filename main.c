/*
    ============================================================
    CROSS ROAD GAME - 보고서 제출용 코드
    ============================================================
    <사용된 주요 C언어 요소>
    1. 배열        : 자동차 여러 대, 게임 맵, 문자열 저장에 사용
    2. 함수        : 화면 출력, 이동, 충돌, 점수, 파일 저장 기능을 분리
    3. 구조체      : Player, Car, Coin 정보를 하나의 자료형으로 묶음
    4. 포인터      : 함수 안에서 Player/Coin 원본 값을 직접 수정하기 위해 사용
    5. 파일 입출력 : 최고 점수를 bestscore.txt 파일에 저장/불러오기
    6. 반복문      : 맵 출력, 자동차 이동, 자동차 충돌 검사 등에 사용
    7. 조건문      : 메뉴 선택, 키 입력, 충돌 여부, 스테이지 클리어 판단에 사용
    8. 난수        : 자동차 위치/방향, 코인 위치를 랜덤으로 정할 때 사용
    9. 키 입력     : _getch(), _kbhit()으로 실시간 조작 구현
    ============================================================
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> // 기본 입출력과 파일 입출력을 사용하기 위한 헤더 
#include <stdlib.h> // rand, srand, system 같은 표준 라이브러리 함수를 사용하기 위한 헤더 
#include <time.h>  // srand(time(NULL))로 난수 시드를 현재 시간으로 설정하기 위한 헤더
#include <conio.h> // _getch(), _kbhit()처럼 키보드 입력을 즉시 처리하기 위한 Windows 전용 헤더
#include <windows.h> // 콘솔 색상, 커서 이동, Sleep, 콘솔 크기 조절 등 Windows API를 사용하기 위한 헤더 
#include <string.h> // strlen, strcpy, strcat, sprintf 등 문자열 처리 함수를 사용하기 위한 헤더 

#define WIDTH 54 // 실제 게임이 진행되는 도로 화면의 가로 크기
#define HEIGHT 10 // 실제 게임이 진행되는 도로 화면의 세로 크기
#define MAX_CARS 20 // 자동차가 최대 몇 대까지 등장할 수 있는지 정한 상수
#define MAX_STAGE 5 // 전체 스테이지 개수. 5스테이지를 클리어하면 게임 클리어 
#define MAX_LIFE 3 // 플레이어의 기본 생명 수 
#define NAME_MAX 20 // 닉네임을 저장할 문자 배열 크기. 실제 입력은 끝 문자 \0 때문에 최대 19글자
#define LEFT_W 12 // 왼쪽 파란 조이콘 영역의 가로 폭 
#define CENTER_W 78 // 가운데 게임 화면 영역의 가로 폭 
#define RIGHT_W 12 // 오른쪽 빨간 조이콘 영역의 가로 폭
#define MENU_W 58 // 메뉴 박스의 가로 폭. 선이 삐뚤어지지 않도록 고정된 값 사용
#define NICK_INPUT_X 51 // 닉네임 입력 커서의 X 좌표 
#define NICK_INPUT_Y 12 // 닉네임 입력 커서의 Y 좌표 
#define ROAD_ICON '.' // 도로를 표시하는 문자 
#define GOAL_ICON '=' // 목표 지점, 즉 도착선을 표시하는 문자 
#define START_ICON '_' // 시작 지점을 표시하는 문자 
#define COIN_ICON '$' // 코인을 표시하는 문자 
#define PLAYER_ICON 'O' // 플레이어를 표시하는 문자 
#define CAR_ICON 'X'// 자동차를 표시하는 문자 

/* 러시아워 시스템 설정
   - 1, 2스테이지: 러시아워 없음
   - 3스테이지: 1번
   - 4스테이지: 2번
   - 5스테이지: 3번
   화면 깜빡임 없이 상단 한 줄에만 상태를 고정 표시한다. */
#define RUSH_BONUS_CARS 3   // 러시아워 때 추가되는 자동차 수
#define RUSH_MOVE_COUNT 2   // 러시아워 때 자동차 이동 횟수
#define RUSH_WAIT 4         // 러시아워 시작 전 대기 시간
#define RUSH_DURATION 6     // 러시아워 유지 시간

   /* 최고 점수를 저장하고 불러올 파일 이름 */
#define BEST_SCORE_FILE "bestscore.txt"

/* 왼쪽 조이콘 색상: 파란 배경 + 흰 글자 */
#define COLOR_LEFT   (BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
/* 오른쪽 조이콘 색상: 빨간 배경 + 흰 글자 */
#define COLOR_RIGHT  (BACKGROUND_RED  | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
/* 기본 흰색 글자 */
#define COLOR_WHITE  (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
/* 빨간 글자: 자동차, 충돌, 경고 표시 */
#define COLOR_RED    (FOREGROUND_RED | FOREGROUND_INTENSITY)
/* 초록 글자: 목표 지점 표시 */
#define COLOR_GREEN  (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
/* 노란 글자: 코인 표시 */
#define COLOR_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
/* 하늘색 글자: 플레이어 표시 */
#define COLOR_CYAN   (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
/* 회색 글자: 시작선 표시 */
#define COLOR_GRAY   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

/*
    [구조체 Player]
    플레이어 정보를 하나로 묶은 사용자 정의 자료형이다.
    위치, 생명, 점수, 스테이지, 닉네임을 한 번에 관리할 수 있다.
*/
typedef struct {
    int x;                  /* 플레이어의 가로 위치 */
    int y;                  /* 플레이어의 세로 위치 */
    int life;               /* 플레이어의 남은 생명 */
    int score;              /* 현재 점수 */
    int stage;              /* 현재 스테이지 */
    char name[NAME_MAX];    /* 닉네임을 저장하는 문자 배열 */
} Player;

/*
    [구조체 Car]
    자동차 한 대의 위치와 이동 방향을 저장한다.
*/
typedef struct {
    int x;      /* 자동차의 가로 위치 */
    int y;      /* 자동차의 세로 위치 */
    int dir;    /* 이동 방향: 1이면 오른쪽, -1이면 왼쪽 */
} Car;

/*
    [구조체 Coin]
    코인의 위치를 저장한다.
*/
typedef struct {
    int x;      /* 코인의 가로 위치 */
    int y;      /* 코인의 세로 위치 */
} Coin;

/* 전역 변수: 여러 함수에서 공통으로 사용하는 게임 상태 값 */
int bestScore = 0;      /* 파일에서 불러오거나 새로 저장할 최고 점수 */
int carCount = 0;       /* 현재 스테이지에서 사용되는 자동차 개수 */
int gameRunning = 0;    /* 게임 진행 여부. 1이면 진행 중, 0이면 종료 */
int gameClear = 0;      /* 최종 스테이지 클리어 여부 */

/* 러시아워 관련 전역 변수 */
int baseCarCount = 0;   /* 러시아워가 아닐 때의 기본 자동차 수 */
int rushMode = 0;       /* 러시아워 진행 여부. 1이면 러시아워 */
int rushTick = 0;       /* 러시아워 시작 전 대기 시간 카운트 */
int rushCount = 0;      /* 현재 스테이지에서 발생한 러시아워 횟수 */
int rushRemain = 0;     /* 러시아워 남은 시간 */

void setConsoleSize();
void gotoxy(int x, int y);
void hideCursor();
void showCursor();
void setColor(int color);
void clearScreen();
void clearInputBuffer();

void printRepeat(char ch, int count);
void printFixedText(const char* text, int width);
void printJoycon(int row, int rightSide);
void beginRow(int row);
void endRow(int row);

void printCenterBorder(char ch);
void printCenterText(const char* text);
void printBoxBorder(const char* title);
void printBoxLine(const char* text);
void printBoxBlank();
void printNicknameLine();

void printStatusLine(Player* p);
void printTrafficLine(Player* p);
void printGameBorder();
void printGameRow(char map[HEIGHT][WIDTH + 1], int y);
void printGameChar(char ch);

void drawMenu();
void drawHelp();
void drawNameInput(Player* p);
void drawStageReady(Player* p);
void drawCrash();
void drawResult(Player* p, int newBest);

void loadBestScore();
void saveBestScore();

void resetPlayer(Player* p);
void setCarCount(Player* p);
void initCars(Player* p, Car cars[]);
int isCarPosition(Car cars[], int x, int y);
void initCoin(Coin* coin, Car cars[]);
void initGame(Player* p, Car cars[], Coin* coin);

void drawMap(Player* p, Car cars[], Coin* coin);
void movePlayer(Player* p, char input);
void moveCars(Car cars[]);
int checkCollision(Player* p, Car cars[]);
void checkCoin(Player* p, Coin* coin, Car cars[]);
int checkGoal(Player* p, Car cars[], Coin* coin);
int getGameSpeed(Player* p);

int getRushTargetCount(Player* p);
void resetRushHourStage();
void startRushHour(Car cars[]);
void updateRushHour(Player* p, Car cars[]);

void playGame();

/* 프로그램 시작점이다. 메뉴를 반복 출력하고 사용자의 선택에 따라 기능을 실행한다. */
int main() {
    /* 메뉴 화면 반복 여부를 저장한다. 1이면 계속 실행, 0이면 종료 */
    int running = 1;
    /* 사용자가 메뉴에서 누른 키를 저장한다. */
    char menu;

    /* 화면이 깨지지 않도록 콘솔 크기를 먼저 설정한다. */
    setConsoleSize();
    /* 난수 초기화: 실행할 때마다 자동차와 코인 위치가 다르게 나오게 한다. */
    srand((unsigned int)time(NULL));
    /* 게임 화면에서는 커서가 보이지 않게 숨긴다. */
    hideCursor();
    /* 저장된 최고 점수를 파일에서 불러온다. */
    loadBestScore();

    /* EXIT를 선택하기 전까지 메뉴를 계속 반복한다. */
    while (running == 1) {
        /* 시작 메뉴 화면을 출력한다. */
        drawMenu();
        /* Enter 없이 키 하나를 바로 입력받는다. */
        menu = _getch();

        /* 1번을 누르면 게임을 시작한다. */
        if (menu == '1') {
            playGame();
        }
        /* 2번을 누르면 게임 방법 화면을 보여준다. */
        else if (menu == '2') {
            drawHelp();
            _getch();
        }
        /* 3번 또는 Q를 누르면 프로그램을 종료한다. */
        else if (menu == '3' || menu == 'q' || menu == 'Q') {
            running = 0;
        }
    }

    clearScreen();
    setColor(COLOR_WHITE);
    printf("EXIT GAME.\n");

    return 0;
}

/* 콘솔 창 크기를 고정한다. 게임기 형태 UI가 잘리지 않게 하기 위한 함수이다. */
void setConsoleSize() {
    /* 콘솔 너비/높이를 고정해서 UI가 잘리지 않도록 한다. */
    system("mode con: cols=104 lines=24");
}

/* 콘솔 커서를 특정 좌표(x, y)로 이동한다. 닉네임 입력 위치 지정에 사용한다. */
void gotoxy(int x, int y) {
    /* Windows 콘솔에서 사용할 좌표 구조체를 만든다. */
    COORD pos = { x, y };
    /* 실제로 콘솔 커서를 해당 좌표로 이동한다. */
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/* 게임 화면에서 커서가 깜빡이면 지저분해 보이므로 커서를 숨긴다. */
void hideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

/* 닉네임 입력 때는 사용자가 입력 위치를 알아야 하므로 커서를 다시 보인다. */
void showCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

/* SetConsoleTextAttribute를 이용해 콘솔 글자색/배경색을 변경한다. */
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

/* 콘솔 화면을 지운다. 메뉴 전환, 결과 화면 전환 때 사용한다. */
void clearScreen() {
    /* Windows 명령어 cls로 화면을 지운다. */
    system("cls");
}

/* scanf 입력 후 버퍼에 남아 있는 Enter 값을 제거한다. 다음 입력이 꼬이지 않게 하기 위함이다. */
void clearInputBuffer() {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

/* 같은 문자를 count번 반복 출력한다. 테두리 선을 만들 때 사용한다. */
void printRepeat(char ch, int count) {
    int i;

    /* count만큼 반복해서 같은 문자를 출력한다. */
    for (i = 0; i < count; i++) {
        printf("%c", ch);
    }
}

/* 문자열을 정해진 폭에 맞춰 출력한다. 조이콘 영역의 폭을 항상 일정하게 유지하기 위한 함수이다. */
void printFixedText(const char* text, int width) {
    int i;

    for (i = 0; i < width; i++) {
        if (text[i] != '\0') {
            printf("%c", text[i]);
        }
        else {
            printf(" ");
        }
    }
}

/* 현재 줄(row)에 맞춰 왼쪽/오른쪽 조이콘 그림을 출력한다. rightSide 값으로 어느 쪽 조이콘인지 구분한다. */
void printJoycon(int row, int rightSide) {
    /* 조이콘 안에 출력할 임시 문자열 배열이다. */
    char text[20] = "";

    /* rightSide가 0이면 왼쪽 파란 조이콘을 출력한다. */
    if (rightSide == 0) {
        if (row == 7) {
            sprintf(text, "     ^");
        }
        else if (row == 8) {
            sprintf(text, "   < O >");
        }
        else if (row == 9) {
            sprintf(text, "     v");
        }

        setColor(COLOR_LEFT);
        printFixedText(text, LEFT_W);
    }
    /* 그 외에는 오른쪽 빨간 조이콘을 출력한다. */
    else {
        if (row == 6) {
            sprintf(text, "     W");
        }
        else if (row == 8) {
            sprintf(text, "  A     D");
        }
        else if (row == 10) {
            sprintf(text, "     S");
        }
        else if (row == 14) {
            sprintf(text, "  Q (Quit)");
        }

        setColor(COLOR_RIGHT);
        printFixedText(text, RIGHT_W);
    }
}

/* 한 줄을 출력할 때 먼저 왼쪽 조이콘을 출력한다. */
void beginRow(int row) {
    printJoycon(row, 0);
    setColor(COLOR_WHITE);
}

/* 한 줄의 마지막에 오른쪽 조이콘을 출력하고 줄바꿈한다. */
void endRow(int row) {
    printJoycon(row, 1);
    setColor(COLOR_WHITE);
    printf("\n");
}

/* 중앙 화면의 가로 테두리를 출력한다. */
void printCenterBorder(char ch) {
    printRepeat(ch, CENTER_W);
}

/* 중앙 화면 안에서 문자열을 가운데 정렬해 출력한다. */
void printCenterText(const char* text) {
    /* 출력할 문자열의 길이를 구한다. */
    int len = (int)strlen(text);
    /* 양쪽 테두리 | |를 제외한 실제 내부 폭 */
    int innerWidth = CENTER_W - 2;
    int leftSpace;
    int rightSpace;
    int i;

    if (len > innerWidth) {
        len = innerWidth;
    }

    /* 가운데 정렬을 위해 왼쪽 공백 수를 계산한다. */
    leftSpace = (innerWidth - len) / 2;
    /* 오른쪽 공백 수를 계산한다. */
    rightSpace = innerWidth - len - leftSpace;

    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
    printRepeat(' ', leftSpace);

    for (i = 0; i < len; i++) {
        printf("%c", text[i]);
    }

    printRepeat(' ', rightSpace);
    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
}

/* 메뉴 박스의 위/아래 테두리를 만든다. MENU_W로 폭을 고정해 선이 밀리지 않게 한다. */
void printBoxBorder(const char* title) {
    char line[MENU_W + 1];
    int i;
    int len;
    int start;

    for (i = 0; i < MENU_W; i++) {
        line[i] = '-';
    }

    /* 박스 왼쪽 모서리 */
    line[0] = '+';
    /* 박스 오른쪽 모서리 */
    line[MENU_W - 1] = '+';
    /* 문자열 끝을 표시한다. C 문자열에는 반드시 필요하다. */
    line[MENU_W] = '\0';

    if (title != NULL && strlen(title) > 0) {
        len = (int)strlen(title);

        if (len > MENU_W - 2) {
            len = MENU_W - 2;
        }

        start = (MENU_W - len) / 2;

        for (i = 0; i < len; i++) {
            line[start + i] = title[i];
        }
    }

    printCenterText(line);
}

/* 메뉴 박스 안의 한 줄을 만든다. 텍스트를 박스 내부 중앙에 배치한다. */
void printBoxLine(const char* text) {
    char line[MENU_W + 1];
    int i;
    int len;
    int start;

    for (i = 0; i < MENU_W; i++) {
        line[i] = ' ';
    }

    line[0] = '|';
    line[MENU_W - 1] = '|';
    /* 문자열 끝을 표시한다. C 문자열에는 반드시 필요하다. */
    line[MENU_W] = '\0';

    len = (int)strlen(text);

    if (len > MENU_W - 4) {
        len = MENU_W - 4;
    }

    start = (MENU_W - len) / 2;

    for (i = 0; i < len; i++) {
        line[start + i] = text[i];
    }

    printCenterText(line);
}

/* 메뉴 박스 안에 빈 줄을 출력한다. */
void printBoxBlank() {
    printBoxLine("");
}

/* 닉네임 입력 라인을 출력한다. 실제 입력은 gotoxy로 커서를 이동한 뒤 scanf로 받는다. */
void printNicknameLine() {
    char line[MENU_W + 1];
    const char* label = "NICKNAME : ";
    int i;
    int start = 18;

    for (i = 0; i < MENU_W; i++) {
        line[i] = ' ';
    }

    line[0] = '|';
    line[MENU_W - 1] = '|';
    /* 문자열 끝을 표시한다. C 문자열에는 반드시 필요하다. */
    line[MENU_W] = '\0';

    for (i = 0; label[i] != '\0'; i++) {
        line[start + i] = label[i];
    }

    printCenterText(line);
}

/* 게임 상단 상태창을 출력한다. Player 포인터를 통해 stage/life/score를 읽는다. */
void printStatusLine(Player* p) {
    char line[120];
    char lifeText[20] = "";
    int i;

    for (i = 0; i < MAX_LIFE; i++) {
        if (i < p->life) {
            strcat(lifeText, "<3 ");
        }
        else {
            strcat(lifeText, "   ");
        }
    }

    sprintf(
        line,
        "STAGE %d/%d        LIFE %s       SCORE %d        BEST %d",
        p->stage,
        MAX_STAGE,
        lifeText,
        p->score,
        bestScore
    );

    printCenterText(line);
}


/* 러시아워 상태를 상단에 한 줄로 고정 출력한다. 깜빡임 없이 현재 상태만 보여준다. */
void printTrafficLine(Player* p) {
    char line[120];
    int targetRush = getRushTargetCount(p);

    if (targetRush == 0) {
        setColor(COLOR_GREEN);
        printCenterText("TRAFFIC : NORMAL        RUSH HOUR : NONE");
        setColor(COLOR_WHITE);
    }
    else if (rushMode == 1) {
        setColor(COLOR_RED);
        sprintf(line, "!!! RUSH HOUR MODE !!!        CARS +%d / SPEED x%d",
            RUSH_BONUS_CARS,
            RUSH_MOVE_COUNT
        );
        printCenterText(line);
        setColor(COLOR_WHITE);
    }
    else if (rushCount >= targetRush) {
        setColor(COLOR_GREEN);
        printCenterText("TRAFFIC : NORMAL        RUSH HOUR : FINISHED");
        setColor(COLOR_WHITE);
    }
    else {
        setColor(COLOR_YELLOW);
        sprintf(line, "TRAFFIC : NORMAL        RUSH HOUR %d/%d WAITING",
            rushCount,
            targetRush
        );
        printCenterText(line);
        setColor(COLOR_WHITE);
    }
}

/* 실제 게임 맵을 감싸는 작은 테두리를 출력한다. */
void printGameBorder() {
    int sideSpace = (CENTER_W - 2 - (WIDTH + 2)) / 2;

    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
    printRepeat(' ', sideSpace);
    printf("+");
    printRepeat('-', WIDTH);
    printf("+");
    printRepeat(' ', sideSpace);
    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
}

/* 게임 맵의 문자 하나를 받아 종류별 색상을 적용해 출력한다. */
void printGameChar(char ch) {
    if (ch == PLAYER_ICON) {
        setColor(COLOR_CYAN);
    }
    else if (ch == COIN_ICON) {
        setColor(COLOR_YELLOW);
    }
    else if (ch == CAR_ICON) {
        setColor(COLOR_RED);
    }
    else if (ch == GOAL_ICON) {
        setColor(COLOR_GREEN);
    }
    else if (ch == START_ICON) {
        setColor(COLOR_GRAY);
    }
    /* 그 외에는 오른쪽 빨간 조이콘을 출력한다. */
    else {
        setColor(COLOR_WHITE);
    }

    printf("%c", ch);
    setColor(COLOR_WHITE);
}

/* 2차원 배열 map의 y번째 줄을 출력한다. */
void printGameRow(char map[HEIGHT][WIDTH + 1], int y) {
    int sideSpace = (CENTER_W - 2 - (WIDTH + 2)) / 2;
    int x;

    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
    printRepeat(' ', sideSpace);
    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");

    for (x = 0; x < WIDTH; x++) {
        printGameChar(map[y][x]);
    }

    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
    printRepeat(' ', sideSpace);
    /* 중앙 화면의 왼쪽 테두리를 출력한다. */
    printf("|");
}

/* 시작 메뉴 화면을 출력한다. 게임 시작, 설명 보기, 종료 메뉴를 보여준다. */
void drawMenu() {
    int row;
    char bestLine[80];

    clearScreen();
    sprintf(bestLine, "BEST SCORE : %d", bestScore);

    for (row = 0; row < 21; row++) {
        beginRow(row);

        if (row == 0 || row == 20) {
            printCenterBorder('=');
        }
        else if (row == 2) {
            printCenterText(" #####  #####   ###   #####  #####      #####   ###    ###   #### ");
        }
        else if (row == 3) {
            printCenterText(" #      #   #  #   #  #      #          #   #  #   #  #   #  #   #");
        }
        else if (row == 4) {
            printCenterText(" #      #####  #   #  #####  #####      #####  #   #  #####  #   #");
        }
        else if (row == 5) {
            printCenterText(" #      #  #   #   #      #      #      #  #   #   #  #   #  #   #");
        }
        else if (row == 6) {
            printCenterText(" #####  #   #   ###   #####  #####      #   #   ###   #   #  #### ");
        }
        else if (row == 8) {
            printCenterText("CROSS ROAD GAME");
        }
        else if (row == 10) {
            printBoxBorder(" MENU ");
        }
        else if (row == 11) {
            printBoxBlank();
        }
        else if (row == 12) {
            printBoxLine("1. GAME START");
        }
        else if (row == 13) {
            printBoxLine("2. HOW TO PLAY");
        }
        else if (row == 14) {
            printBoxLine("3. EXIT");
        }
        else if (row == 15) {
            printBoxBlank();
        }
        else if (row == 16) {
            printBoxBorder("");
        }
        else if (row == 18) {
            printCenterText(bestLine);
        }
        else if (row == 19) {
            printCenterText("SELECT MENU NUMBER");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }
}

/* 게임 방법 화면을 출력한다. 조작법, 아이콘 의미, 규칙을 안내한다. */
void drawHelp() {
    int row;

    clearScreen();

    for (row = 0; row < 21; row++) {
        beginRow(row);

        if (row == 0 || row == 20) {
            printCenterBorder('=');
        }
        else if (row == 1) {
            printBoxBorder(" HOW TO PLAY ");
        }
        else if (row == 2) {
            printBoxBlank();
        }
        else if (row == 3) {
            printBoxLine("Goal");
        }
        else if (row == 4) {
            printBoxLine("- Reach the green goal line.");
        }
        else if (row == 5) {
            printBoxLine("- Clear Stage 1 to Stage 5.");
        }
        else if (row == 6) {
            printBoxBlank();
        }
        else if (row == 7) {
            printBoxLine("Controls");
        }
        else if (row == 8) {
            printBoxLine("- W : Move Up       S : Move Down");
        }
        else if (row == 9) {
            printBoxLine("- A : Move Left     D : Move Right");
        }
        else if (row == 10) {
            printBoxBlank();
        }
        else if (row == 12) {
            printBoxLine("Icons");
        }
        else if (row == 13) {
            printBoxLine("- O : Player   X : Car   $ : Coin");
        }
        else if (row == 14) {
            printBoxBlank();
        }
        else if (row == 15) {
            printBoxLine("Rules");
        }
        else if (row == 16) {
            printBoxLine("- If you hit a car, life decreases.");
        }
        else if (row == 17) {
            printBoxLine("- Stage clear gives +10 points.");
        }
        else if (row == 19) {
            printBoxBorder(" PRESS ANY KEY TO MENU ");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }
}

/* 게임 시작 전 닉네임 입력 화면이다. 입력한 이름을 Player 구조체의 name 배열에 저장한다. */
void drawNameInput(Player* p) {
    int row;

    /* 닉네임 입력 실패 시 사용할 기본 이름을 미리 저장한다. */
    strcpy(p->name, "PLAYER");
    clearScreen();

    for (row = 0; row < 21; row++) {
        beginRow(row);

        if (row == 0 || row == 20) {
            printCenterBorder('=');
        }
        else if (row == 2) {
            printCenterText("PLAYER SETTING");
        }
        else if (row == 5) {
            printBoxBorder(" NICKNAME ");
        }
        else if (row == 6) {
            printBoxBlank();
        }
        else if (row == 7) {
            printBoxLine("Enter your nickname before start.");
        }
        else if (row == 8) {
            printBoxLine("Nickname: English only, up to 19 letters");
        }
        else if (row == 9) {
            printBoxBlank();
        }
        else if (row == 12) {
            printNicknameLine();
        }
        else if (row == 14) {
            printBoxBlank();
        }
        else if (row == 15) {
            printBoxBorder("");
        }
        else if (row == 18) {
            printCenterText("PRESS ENTER AFTER TYPING");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }

    /* 닉네임 입력할 때는 커서를 보여준다. */
    showCursor();
    setColor(COLOR_CYAN);
    /* 닉네임 입력 위치로 커서를 이동한다. */
    gotoxy(NICK_INPUT_X, NICK_INPUT_Y);

    /* 닉네임을 최대 19글자까지 입력받아 Player 구조체의 name 배열에 저장한다. */
    if (scanf("%19s", p->name) != 1) {
        strcpy(p->name, "PLAYER");
    }

    /* 입력 뒤 남은 Enter 값을 제거한다. */
    clearInputBuffer();
    setColor(COLOR_WHITE);
    /* 게임 화면에서는 커서가 보이지 않게 숨긴다. */
    hideCursor();
    clearScreen();
}

/* 스테이지 시작 전에 현재 플레이어 이름과 스테이지 번호를 보여준다. */
void drawStageReady(Player* p) {
    int row;
    char stageText[40];
    char nameText[60];

    clearScreen();
    /* 현재 스테이지 번호를 문자열로 만든다. */
    sprintf(stageText, "STAGE %d", p->stage);
    /* 플레이어 닉네임을 출력용 문자열로 만든다. */
    sprintf(nameText, "PLAYER : %s", p->name);

    for (row = 0; row < 19; row++) {
        beginRow(row);

        if (row == 0 || row == 18) {
            printCenterBorder('=');
        }
        else if (row == 2) {
            printCenterText("PIXEL SWITCH");
        }
        else if (row == 5) {
            printCenterText(nameText);
        }
        else if (row == 7) {
            printCenterText(stageText);
        }
        else if (row == 9) {
            printCenterText("GET READY!");
        }
        else if (row == 12) {
            printCenterText("AVOID CARS AND REACH THE GOAL");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }

    Sleep(1000);
    clearScreen();
}

/* 자동차와 충돌했을 때 잠깐 보여주는 CRASH 화면이다. */
void drawCrash() {
    int row;

    clearScreen();

    for (row = 0; row < 19; row++) {
        beginRow(row);

        if (row == 0 || row == 18) {
            printCenterBorder('=');
        }
        else if (row == 6) {
            printCenterText("CRASH!");
        }
        else if (row == 8) {
            printCenterText("LIFE -1");
        }
        else if (row == 11) {
            printCenterText("BACK TO START LINE");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }

    Sleep(700);
    clearScreen();
}

/* 게임 종료 후 결과 화면을 출력한다. 닉네임, 점수, 최고 점수를 보여준다. */
void drawResult(Player* p, int newBest) {
    int row;
    char nameLine[80];
    char scoreLine[80];
    char bestLine[80];

    clearScreen();

    sprintf(nameLine, "PLAYER : %s", p->name);
    sprintf(scoreLine, "SCORE  : %d", p->score);
    sprintf(bestLine, "BEST   : %d", bestScore);

    for (row = 0; row < 19; row++) {
        beginRow(row);

        if (row == 0 || row == 18) {
            printCenterBorder('=');
        }
        else if (row == 4) {
            if (gameClear == 1) {
                printCenterText("GAME CLEAR!");
            }
            /* 나머지 줄은 도로 */
            else {
                printCenterText("GAME OVER");
            }
        }
        else if (row == 7) {
            printCenterText(nameLine);
        }
        else if (row == 9) {
            printCenterText(scoreLine);
        }
        else if (row == 11) {
            printCenterText(bestLine);
        }
        else if (row == 13 && newBest == 1) {
            printCenterText("NEW BEST SCORE SAVED!");
        }
        else if (row == 16) {
            printCenterText("PRESS ANY KEY TO MENU");
        }
        else {
            printCenterText("");
        }

        endRow(row);
    }
}

/* bestscore.txt 파일에서 최고 점수를 읽어오는 파일 입력 함수이다. */
void loadBestScore() {
    /* 최고 점수 파일을 읽기 모드로 연다. 파일 포인터 FILE* 사용 */
    FILE* fp = fopen(BEST_SCORE_FILE, "r");

    /* 파일 열기에 실패했을 때를 대비한 예외 처리 */
    if (fp == NULL) {
        bestScore = 0;
        return;
    }

    if (fscanf(fp, "%d", &bestScore) != 1) {
        bestScore = 0;
    }

    /* 사용한 파일을 닫아 자원을 정리한다. */
    fclose(fp);
}

/* 현재 최고 점수를 bestscore.txt 파일에 저장하는 파일 출력 함수이다. */
void saveBestScore() {
    /* 최고 점수 파일을 쓰기 모드로 연다. 기존 내용은 새 점수로 덮어쓴다. */
    FILE* fp = fopen(BEST_SCORE_FILE, "w");

    /* 파일 열기에 실패했을 때를 대비한 예외 처리 */
    if (fp == NULL) {
        return;
    }

    fprintf(fp, "%d", bestScore);
    /* 사용한 파일을 닫아 자원을 정리한다. */
    fclose(fp);
}

/* 플레이어 위치를 시작선 중앙으로 되돌린다. 포인터로 원본 Player 값을 직접 수정한다. */
void resetPlayer(Player* p) {
    /* 플레이어 X 좌표를 화면 중앙으로 설정한다. */
    p->x = WIDTH / 2;
    /* 플레이어 Y 좌표를 시작선, 즉 맨 아래 줄로 설정한다. */
    p->y = HEIGHT - 1;
}

/* 현재 스테이지에 맞춰 자동차 개수를 정한다. 스테이지가 올라갈수록 자동차 수가 증가한다. */
void setCarCount(Player* p) {
    /* 스테이지가 올라갈수록 자동차 수를 늘린다. */
    carCount = 3 + p->stage * 2;

    /* 자동차 수가 최대값을 넘지 않도록 제한한다. */
    if (carCount > MAX_CARS) {
        carCount = MAX_CARS;
    }

    /* 러시아워가 아닐 때의 기본 자동차 수를 저장한다. */
    baseCarCount = carCount;
}

/* 자동차 구조체 배열을 초기화한다. 각 자동차 위치와 방향을 난수로 정한다. */
void initCars(Player* p, Car cars[]) {
    int i;

    setCarCount(p);

    /* 현재 자동차 수만큼 반복한다. */
    for (i = 0; i < carCount; i++) {
        /* 자동차의 X 위치를 랜덤으로 정한다. */
        cars[i].x = rand() % WIDTH;
        /* 시작선과 목표선을 제외한 도로 영역에 자동차를 배치한다. */
        cars[i].y = 1 + rand() % (HEIGHT - 2);

        /* 자동차 이동 방향을 랜덤으로 결정한다. */
        if (rand() % 2 == 0) {
            /* 오른쪽으로 이동 */
            cars[i].dir = 1;
        }
        else {
            /* 왼쪽으로 이동 */
            cars[i].dir = -1;
        }
    }
}

/* 특정 좌표에 자동차가 있는지 검사한다. 코인 위치가 자동차와 겹치지 않게 하기 위해 사용한다. */
int isCarPosition(Car cars[], int x, int y) {
    int i;

    /* 현재 자동차 수만큼 반복한다. */
    for (i = 0; i < carCount; i++) {
        if (cars[i].x == x && cars[i].y == y) {
            return 1;
        }
    }

    return 0;
}

/* 코인의 위치를 난수로 정한다. Coin 포인터를 통해 원본 코인 좌표를 직접 수정한다. */
void initCoin(Coin* coin, Car cars[]) {
    do {
        /* 코인의 X 위치를 랜덤으로 정한다. */
        coin->x = 1 + rand() % (WIDTH - 2);
        /* 코인의 Y 위치를 도로 영역 안에서 랜덤으로 정한다. */
        coin->y = 1 + rand() % (HEIGHT - 2);
        /* 코인이 자동차와 같은 위치에 생기면 다시 위치를 뽑는다. */
    } while (isCarPosition(cars, coin->x, coin->y) == 1);
}

/* 한 판의 게임을 시작하기 전 플레이어, 자동차, 코인, 상태 변수를 초기화한다. */
void initGame(Player* p, Car cars[], Coin* coin) {
    p->stage = 1;
    p->score = 0;
    p->life = MAX_LIFE;

    /* 게임 진행 상태를 켠다. */
    gameRunning = 1;
    /* 새 게임 시작 시 클리어 상태를 초기화한다. */
    gameClear = 0;

    resetPlayer(p);
    initCars(p, cars);
    initCoin(coin, cars);
    resetRushHourStage();
}

/* 실제 게임 진행 화면을 그린다. 2차원 배열에 게임 요소를 배치한 뒤 출력한다. */
void drawMap(Player* p, Car cars[], Coin* coin) {
    /* 2차원 배열: 현재 게임 화면을 문자로 저장한다. +1은 문자열 끝 문자 공간 */
    char map[HEIGHT][WIDTH + 1];
    int row;
    int y;
    int x;
    int i;

    /* 화면을 매번 지우지 않고 커서를 맨 위로 보내 다시 그린다. */
    gotoxy(0, 0);

    /* 맵의 세로 줄을 반복한다. */
    for (y = 0; y < HEIGHT; y++) {
        /* 맵의 가로 칸을 반복한다. */
        for (x = 0; x < WIDTH; x++) {
            /* 맨 위 줄은 목표 지점 */
            if (y == 0) {
                map[y][x] = GOAL_ICON;
            }
            /* 맨 아래 줄은 시작 지점 */
            else if (y == HEIGHT - 1) {
                map[y][x] = START_ICON;
            }
            /* 나머지 줄은 도로 */
            else {
                map[y][x] = ROAD_ICON;
            }
        }

        map[y][WIDTH] = '\0';
    }

    /* 코인의 위치에 코인 문자를 넣는다. */
    map[coin->y][coin->x] = COIN_ICON;

    /* 현재 자동차 수만큼 반복한다. */
    for (i = 0; i < carCount; i++) {
        /* 자동차 위치에 자동차 문자를 넣는다. */
        map[cars[i].y][cars[i].x] = CAR_ICON;
    }

    /* 플레이어 위치에 플레이어 문자를 넣는다. */
    map[p->y][p->x] = PLAYER_ICON;

    row = 0;

    beginRow(row);
    printCenterBorder('=');
    endRow(row++);

    beginRow(row);
    printCenterText("PIXEL SWITCH");
    endRow(row++);

    beginRow(row);
    printCenterText("MODE : CROSSY ROAD");
    endRow(row++);

    beginRow(row);
    printStatusLine(p);
    endRow(row++);

    beginRow(row);
    printTrafficLine(p);
    endRow(row++);

    beginRow(row);
    printCenterBorder('-');
    endRow(row++);

    beginRow(row);
    printGameBorder();
    endRow(row++);

    /* 맵의 세로 줄을 반복한다. */
    for (y = 0; y < HEIGHT; y++) {
        beginRow(row);
        printGameRow(map, y);
        endRow(row++);
    }

    beginRow(row);
    printGameBorder();
    endRow(row++);

    beginRow(row);
    printCenterText("PIXEL CONSOLE");
    endRow(row++);

    beginRow(row);
    printCenterBorder('=');
    endRow(row++);

    setColor(COLOR_WHITE);
    /* 출력 버퍼를 비워 화면에 즉시 반영되도록 한다. */
    fflush(stdout);
}

/* 키 입력에 따라 플레이어 위치를 이동한다. Player 포인터를 사용해 원본 좌표를 직접 변경한다. */
void movePlayer(Player* p, char input) {
    /* W 입력: 위로 이동. 화면 밖으로 나가지 않도록 y > 0 조건을 둔다. */
    if ((input == 'w' || input == 'W') && p->y > 0) {
        p->y--;
    }
    /* S 입력: 아래로 이동. 맨 아래를 넘지 않도록 제한한다. */
    else if ((input == 's' || input == 'S') && p->y < HEIGHT - 1) {
        p->y++;
    }
    /* A 입력: 왼쪽으로 이동 */
    else if ((input == 'a' || input == 'A') && p->x > 0) {
        p->x--;
    }
    /* D 입력: 오른쪽으로 이동 */
    else if ((input == 'd' || input == 'D') && p->x < WIDTH - 1) {
        p->x++;
    }
    /* Q 입력: 게임 종료 */
    else if (input == 'q' || input == 'Q') {
        gameRunning = 0;
    }
}

/* 모든 자동차를 현재 방향(dir)으로 한 칸씩 이동시킨다. */
void moveCars(Car cars[]) {
    int i;

    /* 현재 자동차 수만큼 반복한다. */
    for (i = 0; i < carCount; i++) {
        /* 자동차를 자신의 방향(dir)으로 한 칸 이동시킨다. */
        cars[i].x = cars[i].x + cars[i].dir;

        /* 왼쪽 화면 밖으로 나가면 */
        if (cars[i].x < 0) {
            /* 오른쪽 끝에서 다시 등장 */
            cars[i].x = WIDTH - 1;
        }
        /* 오른쪽 화면 밖으로 나가면 */
        else if (cars[i].x >= WIDTH) {
            /* 왼쪽 끝에서 다시 등장 */
            cars[i].x = 0;
        }
    }
}

/* 플레이어와 자동차가 같은 좌표에 있는지 검사한다. 충돌하면 생명을 줄인다. */
int checkCollision(Player* p, Car cars[]) {
    int i;

    /* 현재 자동차 수만큼 반복한다. */
    for (i = 0; i < carCount; i++) {
        /* 플레이어와 자동차 좌표가 같으면 충돌 */
        if (p->x == cars[i].x && p->y == cars[i].y) {
            /* 충돌했으므로 생명 감소 */
            p->life--;

            /* 생명이 0 이하이면 게임 종료 */
            if (p->life <= 0) {
                gameRunning = 0;
            }
            /* 나머지 줄은 도로 */
            else {
                /* 생명이 남아 있으면 시작 위치로 되돌린다. */
                resetPlayer(p);
            }

            return 1;
        }
    }

    return 0;
}

/* 플레이어가 코인 위치에 도착했는지 검사한다. 코인을 먹으면 점수를 올리고 새 코인을 만든다. */
void checkCoin(Player* p, Coin* coin, Car cars[]) {
    /* 플레이어와 코인의 좌표가 같으면 코인 획득 */
    if (p->x == coin->x && p->y == coin->y) {
        /* 코인 점수 5점 추가 */
        p->score += 5;
        initCoin(coin, cars);
    }
}

/* 플레이어가 목표 지점에 도착했는지 검사한다. 도착하면 스테이지를 올리거나 게임을 클리어한다. */
int checkGoal(Player* p, Car cars[], Coin* coin) {
    /* 플레이어가 맨 위 목표선에 도착했는지 확인 */
    if (p->y == 0) {
        /* 스테이지 클리어 보상 점수 */
        p->score += 10;

        /* 마지막 스테이지면 게임 클리어 */
        if (p->stage == MAX_STAGE) {
            gameClear = 1;
            gameRunning = 0;
        }
        else {
            /* 다음 스테이지로 이동 */
            p->stage++;
            resetPlayer(p);
            initCars(p, cars);
            initCoin(coin, cars);
            resetRushHourStage();
            return 1;
        }
    }

    return 0;
}


/* 현재 스테이지에서 러시아워가 몇 번 발생할지 정한다.
   1, 2스테이지는 0번 / 3스테이지는 1번 / 4스테이지는 2번 / 5스테이지는 3번 */
int getRushTargetCount(Player* p) {
    if (p->stage < 3) {
        return 0;
    }

    return p->stage - 2;
}

/* 새 스테이지가 시작될 때 러시아워 상태를 초기화한다. */
void resetRushHourStage() {
    rushMode = 0;
    rushTick = 0;
    rushCount = 0;
    rushRemain = 0;
    carCount = baseCarCount;
}

/* 러시아워를 시작한다. 추가 자동차를 배치하고 자동차 수를 늘린다. */
void startRushHour(Car cars[]) {
    int i;
    int newCount;

    rushMode = 1;
    rushRemain = RUSH_DURATION;
    rushCount++;

    newCount = baseCarCount + RUSH_BONUS_CARS;

    if (newCount > MAX_CARS) {
        newCount = MAX_CARS;
    }

    /* 러시아워 때 추가되는 자동차만 새로 초기화한다. */
    for (i = baseCarCount; i < newCount; i++) {
        cars[i].x = rand() % WIDTH;
        cars[i].y = 1 + rand() % (HEIGHT - 2);

        if (rand() % 2 == 0) {
            cars[i].dir = 1;
        }
        else {
            cars[i].dir = -1;
        }
    }

    carCount = newCount;
}

/* 매 반복마다 러시아워 시작/종료 상태를 갱신한다. */
void updateRushHour(Player* p, Car cars[]) {
    int targetRush;

    targetRush = getRushTargetCount(p);

    /* 1, 2스테이지는 러시아워가 없다. */
    if (targetRush == 0) {
        rushMode = 0;
        rushTick = 0;
        rushCount = 0;
        rushRemain = 0;
        carCount = baseCarCount;
        return;
    }

    /* 러시아워 진행 중이면 유지 시간을 줄이고, 끝나면 기본 자동차 수로 되돌린다. */
    if (rushMode == 1) {
        rushRemain--;

        if (rushRemain <= 0) {
            rushMode = 0;
            rushTick = 0;
            carCount = baseCarCount;
        }

        return;
    }

    /* 해당 스테이지의 러시아워 횟수를 모두 사용했으면 더 이상 발생하지 않는다. */
    if (rushCount >= targetRush) {
        return;
    }

    /* 일정 시간 대기 후 러시아워를 시작한다. */
    rushTick++;

    if (rushTick >= RUSH_WAIT) {
        startRushHour(cars);
    }
}

/* 현재 스테이지에 따라 게임 속도를 계산한다. Sleep 시간으로 사용된다. */
int getGameSpeed(Player* p) {
    int speed;

    /* 스테이지가 높을수록 Sleep 시간이 짧아져 속도가 빨라진다. */
    speed = 300 - p->stage * 25;

    /* 러시아워 중에는 화면 갱신 속도도 조금 더 빠르게 한다. */
    if (rushMode == 1) {
        speed = speed - 40;
    }

    /* 너무 빨라져서 게임이 깨지지 않도록 최소값을 둔다. */
    if (speed < 80) {
        speed = 80;
    }

    return speed;
}

/* 실제 게임 한 판의 전체 흐름을 관리한다. 입력, 이동, 충돌, 점수, 결과 저장을 모두 연결한다. */
void playGame() {
    /* Player 구조체 변수: 한 판의 플레이어 정보 저장 */
    Player player;
    /* Car 구조체 배열: 여러 자동차 정보를 저장 */
    Car cars[MAX_CARS];
    /* Coin 구조체 변수: 현재 코인 위치 저장 */
    Coin coin;

    int newBest = 0;
    int crash;
    int stageUp;
    int moveStep;

    /* Player 구조체 주소를 넘겨 닉네임을 입력받는다. */
    drawNameInput(&player);
    /* Player, Car 배열, Coin을 초기화한다. */
    initGame(&player, cars, &coin);
    /* 스테이지 시작 안내 화면 출력 */
    drawStageReady(&player);

    /* 게임이 진행 중일 동안 계속 반복한다. */
    while (gameRunning == 1) {
        /* 러시아워 상태를 먼저 갱신한 뒤 화면에 출력한다. */
        updateRushHour(&player, cars);

        /* 현재 게임 상태를 화면에 출력한다. */
        drawMap(&player, cars, &coin);

        /* 키 입력이 들어왔는지 확인한다. */
        if (_kbhit()) {
            /* 입력된 키를 Enter 없이 바로 읽는다. */
            char input = _getch();
            /* 입력 키에 따라 플레이어를 이동시킨다. */
            movePlayer(&player, input);
        }

        /* 모든 자동차를 이동시킨다.
           러시아워 중에는 자동차를 2번 이동시켜 속도가 빨라진 것처럼 만든다. */
        crash = 0;

        for (moveStep = 0; moveStep < RUSH_MOVE_COUNT; moveStep++) {
            if (rushMode == 0 && moveStep > 0) {
                break;
            }

            moveCars(cars);

            /* 자동차와 충돌했는지 확인한다. */
            crash = checkCollision(&player, cars);

            if (crash == 1) {
                break;
            }
        }

        if (crash == 1) {
            if (gameRunning == 1) {
                /* 충돌 안내 화면을 보여준다. */
                drawCrash();
            }

            continue;
        }

        /* 코인 획득 여부를 확인한다. */
        checkCoin(&player, &coin, cars);

        /* 목표선 도착 여부와 스테이지 클리어를 확인한다. */
        stageUp = checkGoal(&player, cars, &coin);

        if (stageUp == 1 && gameRunning == 1) {
            /* 다음 스테이지 시작 화면을 보여준다. */
            drawStageReady(&player);
        }

        /* 계산된 속도만큼 잠시 멈춰 게임 속도를 조절한다. */
        Sleep(getGameSpeed(&player));
    }

    /* 현재 점수가 최고 점수보다 높으면 */
    if (player.score > bestScore) {
        /* 최고 점수를 갱신한다. */
        bestScore = player.score;
        /* 갱신된 최고 점수를 파일에 저장한다. */
        saveBestScore();
        newBest = 1;
    }

    /* 결과 화면을 출력한다. */
    drawResult(&player, newBest);
    _getch();
    clearScreen();
}

