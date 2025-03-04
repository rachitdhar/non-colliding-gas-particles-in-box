#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <windows.h>
#include <stdlib.h>

const wchar_t BLOCK = L'\u2588';
const wchar_t CIRCLE = L'\u2b24';
const wchar_t* RED_COLOR = L"\x1b[0;31m";
const wchar_t* CYAN_COLOR = L"\x1b[0;96m";
const wchar_t* RESET_COLOR = L"\x1b[0;0m";

const int BOUND_LENGTH = 60;
const int BOUND_WIDTH = 20;
const auto TIME_INTERVAL = std::chrono::milliseconds(50);

void moveCursorToStart() { wprintf(L"\033[G"); }
void displayBlock() { wprintf(L"%lc", BLOCK); }
void moveCursorToPosition(int row, int col) { wprintf(L"\033[%d;%dH", row, col); }
void clearConsole() { moveCursorToPosition(0, 0); wprintf(L"\033[J"); }

void hBar(int numChars, int rowPosition, int startColumn)
{
    for (size_t i = 0; i < numChars; i++) {
        moveCursorToPosition(rowPosition, startColumn + (int)i);
        displayBlock();
    }
}

void vBar(int numChars, int startRow, int columnPosition)
{
    for (size_t i = 0; i < numChars; i++) {
        moveCursorToPosition(startRow + (int)i, columnPosition);
        displayBlock();
        wprintf(L"\n");
    }
}

float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

struct Particle
{
    float Row = RandomFloat(3.0, BOUND_WIDTH - 1.0);
    float Col = RandomFloat(3.0, BOUND_LENGTH - 1.0);
    float PrevRow = -1;
    float PrevCol = -1;
    float RSpeed = 0;
    float CSpeed = 0;
    const float INIT_RSPEED = RandomFloat(-1.0, 1.0);
    const float INIT_CSPEED = RandomFloat(-1.0, 1.0);
};

class Gas
{
public:
    int NumberOfParticles = 20;
    std::vector<Particle> ParticleGroup;

    Gas(int number) {
        for (int i = 0; i < number; i++) {
            Particle p;
            p.RSpeed = p.INIT_RSPEED;
            p.CSpeed = p.INIT_CSPEED;
            this->ParticleGroup.push_back(p);
        }
        this->NumberOfParticles = number;
    }

    void AddParticle(int row, int col)
    {
        Particle p;
        p.RSpeed = p.INIT_RSPEED;
        p.CSpeed = p.INIT_CSPEED;
        p.Row = row;
        p.Col = col;
        this->ParticleGroup.push_back(p);
        this->NumberOfParticles++;
    }
};

void clearParticle(int row, int col) { moveCursorToPosition(row, col); wprintf(L" "); }

void performAction(Particle* particle) {
    if ((particle->Row <= 3.0 && particle->RSpeed < 0.0) || (particle->Row >= BOUND_WIDTH - 1 && particle->RSpeed > 0.0)) particle->RSpeed *= -1;
    else if ((particle->Col <= 3.0 && particle->CSpeed < 0.0) || (particle->Col >= BOUND_LENGTH - 1 && particle->CSpeed > 0.0)) particle->CSpeed *= -1;
}

void displayParticleAtPos(int row, int col, const wchar_t* color)
{
    moveCursorToPosition(row, col);
    wprintf(L"%s%lc%s", color, CIRCLE, RESET_COLOR);
}

void moveParticle(Particle* particle)
{
    displayParticleAtPos((int)(particle->Row), (int)(particle->Col), CYAN_COLOR);
    particle->PrevRow = particle->Row;
    particle->PrevCol = particle->Col;
    particle->Row += particle->RSpeed;
    particle->Col += particle->CSpeed;
}

void renderConsole(Gas* gas)
{
    for (Particle &p : gas->ParticleGroup)
    {
        if (p.PrevRow != -1) clearParticle((int)p.PrevRow, (int)p.PrevCol);
        moveParticle(&p);
    }
}


int main()
{
    _setmode(_fileno(stdout), _O_U8TEXT);

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(h, &cursorInfo);
    cursorInfo.bVisible = false; // set the cursor visibility
    SetConsoleCursorInfo(h, &cursorInfo);
    // Creating boundaries
    hBar(BOUND_LENGTH, 0, 0);
    hBar(BOUND_LENGTH + 1, BOUND_WIDTH, 0);
    vBar(BOUND_WIDTH, 0, 0);
    vBar(BOUND_WIDTH, 0, BOUND_LENGTH);

    int NumParticles = 10;
    Gas gas(NumParticles);

    int count = 0;
    while (1)
    {
        count++;
        std::this_thread::sleep_for(TIME_INTERVAL);
        renderConsole(&gas);
        
        for (Particle &p : gas.ParticleGroup) performAction(&p);
        if (count > 1000) break;
    }
    return 0;
}
