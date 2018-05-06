/*
 * MAZE Game Framework
 * Written by Dr. Dhanyu Amarasinghe Spring 2018
 */

#include <string>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <Timer.h>
#include <player.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <Enemies.h>

#include <wall.h>
#include <math.h>

#include<fstream>
#include<cmath>
#include<iomanip>
#include <queue>
//#include <node.h>

/* Node structure creation for adjacency list*/
struct node{

       struct node *next;
        int i;
        int j;
        node()
        {
            next = nullptr;
        }
};


struct path{
        int i;
        int j;
        path* parent;
        path()
        {
            i = -1;
            j = -1;
        }
};
/* GLUT callback Handlers */

using namespace std;

Maze *M = new Maze(13);                         // Set Maze grid size
Player *P = new Player();                       // create player

wall W[100];                                    // wall with number of bricks
Enemies E[4];                                  // create number of enemies
Timer *T0 = new Timer();                        // animation timer
char myArray[13][13];

float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping

int plyX;
int plyY;

int enemyCounter = 0; //So you can't place more than 4 enemies
bool playerPlaced = false; //So you can't insert the player twice
int wallCounter = 1; // Tracks the number of walls
int arrowsPlaced = 0; //Have arrows been placed?
bool chestPlaced = false; //Has the chest been placed?


int enemiesLeft = 4;

/* All BFS elements*/
node *adjList[13][13];
bool visited[13][13];
path pathway[500];
path En1, En2, En3, En4;
queue<path> Queue;

/* Game win/lose booleans */
bool isAlive = true;
bool isWin = false;



/* Function Prototypes */
void display(void);  // Main Display : this runs in a loop
void displayMaze();
void addEdges();
void BFS(path*);

void resize(int width, int height)              // resizing case on the window
{
    wWidth = width;
    wHeight = height;

    if(width<=height)
        glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
        glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);
}

void init()
{
    glEnable(GL_COLOR_MATERIAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);

    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    M->loadBackgroundImage("images/bak.jpg");           // Load maze background image

    P->initPlayer(M->getGridSize(),6,"images/p.png");   // initialize player pass grid size,image and number of frames
    P->loadArrowImage("images/arr.png");                // Load arrow image


    //---------------------------------- PUT ELEMENTS INTO THE MAZE FROM TEXT FILE ---------------------------------//
    ifstream indata("\mazeLayout.txt"); //that's my path,you should use yours.
    if(!indata)
    {
        cout<<"No file."<< endl;
    }

    if(indata.is_open())
    {
        for(int i = 0; i < 13; ++i)
        {
            for(int j = 0; j < 13; ++j)
            {
                indata >> myArray[i][j];
                //cout << myArray[i][j];
            }
        }


    }

    for(int i = 0; i < 13; ++i)
    {
        for(int j = 0; j < 13; ++j)
        {
            if (myArray[i][j] == 'X')
            {
                if (enemyCounter < 4)
                {

                    E[enemyCounter].initEnm(M->getGridSize(),4,"images/e.png"); //Load enemy image
                    E[enemyCounter].placeEnemy(j,12-i);
                    enemyCounter++;
                }
                else
                    myArray[i][j] = 'O';
            }

            if (myArray[i][j] == 'P')
            {
                if (!playerPlaced)
                {
                    P->placePlayer(j,12-i);
                    plyY = j;
                    plyX = i;
                    playerPlaced = true;
                }
                else
                    myArray[i][j] = 'O';
            }

            if (myArray[i][j] == '~')
            {
                    W[wallCounter].wallInit(M->getGridSize(),"images/wall.png");// Load walls
                    W[wallCounter].placeWall(j, 12-i);
                    wallCounter++;
            }

            if (myArray[i][j] == 'A')
            {
                if (arrowsPlaced < 10)
                {
                    M->loadSetOfArrowsImage("images/arrwset.png");      // load set of arrows image
                    M->placeStArrws(j,12-i);
                    arrowsPlaced++;
                }
                else
                    myArray[i][j] = 'O';
            }

            if (myArray[i][j] == 'C')
            {
                if (!chestPlaced)
                {
                    M->loadChestImage("images/chest.png");              // load chest image
                    M->placeChest(j,12-i);
                    chestPlaced = true;
                }
                else
                    myArray[i][j] = 'O';
            }
        }
    }
    addEdges();
    displayMaze();

    //----------------------------------Set visited nodes to false ----------------------------//
    for (int i = 0; i < 13; i++)
    {
        for (int j=0; j < 13; j++)
        {
            visited[i][j] = false;
        }
    }


    En1.i = 12-E[0].getEnemyLoc().y;
    En1.j = E[0].getEnemyLoc().x;

    cout << endl << "(" << En1.i << "," << En1.j << ") ->";

}

void addEdges()
{
    node *tail = new node();
    node *temp = new node();
    for (int i = 0; i < 13; i++)
    {

        for(int j = 0; j < 13; j++)
        {
            adjList[i][j] = new node();
            adjList[i][j]->i = i;
            adjList[i][j]->j = j;

            tail = adjList[i][j];
            cout << endl << "(" << i << "," << j << ") ->";
            if (myArray[i-1][j] != '~' && i > 0) //if there is a wall or you are at the edge then don't add it to the adjacency list
            {
                tail->next = new node();
                tail = tail->next;
                tail->i = i-1;
                tail->j = j;
                cout << "(" << tail->i << "," << tail->j << ") ->";
            }
            if (myArray[i+1][j] != '~' && i < 12)
            {
                tail->next = new node();
                tail = tail->next;
                tail->i = i+1;
                tail->j = j;
                cout << "(" << tail->i << "," << tail->j << ") ->";
            }
            if (myArray[i][j-1] != '~' && j > 0)
            {
                tail->next = new node();
                tail = tail->next;
                tail->i = i;
                tail->j = j-1;
                cout << "(" << tail->i << "," << tail->j << ") ->";
            }
            if (myArray[i][j+1] != '~' && j < 12)
            {
                tail->next = new node();
                tail = tail->next;
                tail->i = i;
                tail->j = j+1;
                cout << "(" << tail->i << "," << tail->j << ") ->";

            }
            tail->next = nullptr;
        }
    }
}

void BFS(path s)
{
    Queue.push(s);

    path x;
    int n=0;
    while (!Queue.empty())
    {
        x = Queue.front();
        cout << "(" << x.i << ", " << x.j << ")" << endl;
        Queue.pop();

        node* temp = adjList[x.i][x.j];

        cout << " Back" << endl;
        while(temp != nullptr)
        {
            if(!visited[temp->i][temp->j])
            {
                visited[temp->i][temp->j] = true;
                path temp_path;
                temp_path.i = temp->i;
                temp_path.j = temp->j;



                cout << "(" << temp_path.i << ", " << temp_path.j << ")" << endl;
                Queue.push(temp_path);
            }
            temp = temp->next;
        }
    }
}

void displayPath()
{
    cout << endl << endl << endl << endl << "Pathway:" << endl;
    for (int i = 0; i < 500; i++)
    {
        cout << "(" << pathway[i].i << "," << pathway[i].j << ") -> ";

        if(pathway[i].i == -1)
            return;
    }

    cout << endl << endl << endl ;
}

void display(void)
{
  glClear (GL_COLOR_BUFFER_BIT);        // clear display screen

        if(isWin)
        {
            M->loadBackgroundImage("images/winScrn.png");

            glPushMatrix();
                M->drawBackground();
            glPopMatrix();
        }

        else if (!isAlive)
        {
            M->loadBackgroundImage("images/lose.png");

            glPushMatrix();
                M->drawBackground();
            glPopMatrix();
        }

        else
        {
            glPushMatrix();
            M->drawBackground();
            glPopMatrix();

            for(int i=0; i<wallCounter;i++)
            {
               W[i].drawWall();
            }

            glPushMatrix();
                M->drawGrid();
            glPopMatrix();

            glPushMatrix();
                P->drawplayer();
            glPopMatrix();

            for(int i=0; i<enemyCounter;i++)
            {
            E[i].drawEnemy();
            }

            glPushMatrix();
                P->drawArrow();
            glPopMatrix();

             glPushMatrix();
               M->drawChest();
            glPopMatrix();

            glPushMatrix();
            if(!P->hasArrows)
               M->drawArrows();
            glPopMatrix();

        }


    glutSwapBuffers();
}



 void idle()
{

    glutPostRedisplay();
}

void displayMaze()
{
    /*system("CLS");
    for(int i = 0; i < 13; ++i)
    {
        for(int j = 0; j < 13; ++j)
        {
            cout << myArray[i][j] << " ";
        }
        cout << endl;
    }*/
    glutPostRedisplay();
}

void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case ' ':
          if(P->hasArrows && P->arrowCount > 0)      // if setof arrows were picked by player
             {
                P->shootArrow();
                P->arrowCount--;

             }
        break;
        case 27 :                       // esc key to exit
        case 'q':
            exit(0);
            break;

        case 'w':
            P->updateDirection("up");
        break;

        case 'a':
            P->updateDirection("left");
        break;

        case 's':
            P->updateDirection("down");
        break;

        case 'd':
            P->updateDirection("right");
        break;

        case 'm':
            idle();
        break;
    }

    glutPostRedisplay();
}


 void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    xPos =posX ;
    yPos =posY ;
}


void mouse(int btn, int state, int x, int y){

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;


      case GLUT_RIGHT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;
    }
     glutPostRedisplay();
};

void Specialkeys(int key, int x, int y)
{

    // Your Code here
    switch(key)
    {
    case GLUT_KEY_UP:
        BFS(En1);
        displayPath();
        if (myArray[12-P->getPlayerLoc().y-1][(P->getPlayerLoc().x)] != '~')
        {
            P->movePlayer("up");
            if(plyY > 0)
            {
                int temp = plyY-1;
                if (!(myArray[temp][plyX] == 'X'))
                {
                myArray[plyY][plyX] = 'O';
                plyY--;
                myArray[plyY][plyX] = 'P';
                }

            }
        }

        displayMaze();
         //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;

    break;

    case GLUT_KEY_DOWN:
        BFS(En1);
        if (myArray[12-P->getPlayerLoc().y+1][(P->getPlayerLoc().x)] != '~')
        {
            P->movePlayer("down");

            if(plyY < 12)
            {
                int temp = 1+plyY;
                if (!(myArray[temp][plyX] == 'X'))
                {
                myArray[plyY][plyX] = 'O';
                plyY++;
                myArray[plyY][plyX] = 'P';
                }


            }
        }
        displayMaze();
         //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;

    break;

    case GLUT_KEY_LEFT:
        BFS(En1);
        if (myArray[12-P->getPlayerLoc().y][(P->getPlayerLoc().x-1)] != '~')
        {
            P->movePlayer("left");
            if(plyX > 0)
            {
                int temp = plyX-1;
                if (!(myArray[plyY][temp] == 'X'))
                {
                myArray[plyY][plyX] = 'O';
                plyX--;
                myArray[plyY][plyX] = 'P';
                }

            }
        }
        displayMaze();
         //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;

    break;

    case GLUT_KEY_RIGHT:
        BFS(En1);
        if (myArray[12-P->getPlayerLoc().y][(P->getPlayerLoc().x+1)] != '~')
        {
            P->movePlayer("right");
            if(plyX < 12)
            {
                int temp = 1+plyX;
                if (!(myArray[plyY][temp] == 'X'))
                {
                myArray[plyY][plyX] = 'O';
                plyX++;
                myArray[plyY][plyX] = 'P';
                }

            }
        }
        displayMaze();
         //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
    break;



   }
  glutPostRedisplay();
}

void updateStatus()
{

    //---------------------------- ENEMY WITH ARROW COLLISION -------------------------------------//
    if (P->getArrowLoc().x == E[0].getEnemyLoc().x && P->getArrowLoc().y == E[0].getEnemyLoc().y && E[0].live)
    {
        myArray[12-E[0].getEnemyLoc().y][E[0].getEnemyLoc().x] = 'O';
        enemiesLeft--;
        P->arrowStatus = false;
        E[0].live = 0;
        displayMaze();
    }


    if (P->getArrowLoc().x == E[1].getEnemyLoc().x && P->getArrowLoc().y == E[1].getEnemyLoc().y && E[1].live)
    {
        myArray[12-E[1].getEnemyLoc().y][E[1].getEnemyLoc().x] = 'O';
        enemiesLeft--;
        P->arrowStatus = false;
        E[1].live = 0;
        displayMaze();
    }

    if (P->getArrowLoc().x == E[2].getEnemyLoc().x && P->getArrowLoc().y == E[2].getEnemyLoc().y && E[2].live)
    {
        myArray[12-E[2].getEnemyLoc().y][E[2].getEnemyLoc().x] = 'O';
        enemiesLeft--;
        P->arrowStatus = false;
        E[2].live = 0;
        displayMaze();
    }

    if (P->getArrowLoc().x == E[3].getEnemyLoc().x && P->getArrowLoc().y == E[3].getEnemyLoc().y && E[3].live)
    {
        myArray[12-E[3].getEnemyLoc().y][E[3].getEnemyLoc().x] = 'O';
        enemiesLeft--;
        P->arrowStatus = false;
        E[3].live = 0;
        displayMaze();
    }

    //----------------------------- CHECK IF PICKED UP ARROW ------------------------------------------//
    if(P->getPlayerLoc().x == M->GetStArrwsLoc().x && P->getPlayerLoc().y == M->GetStArrwsLoc().y && !P->arrowStatus )
    {
        P->hasArrows = true;
    }

    //--------------------------------CHECK IF ARROW HITS WALL -----------------------------------------//
    if(myArray[12-P->getArrowLoc().y][P->getArrowLoc().x] == '~')
    {
        P->arrowStatus = false;
    }



    //------------------------------ DIE FROM ENEMY ------------------------------//
    if(myArray[12-P->getPlayerLoc().y][(P->getPlayerLoc().x)] == 'X')
        isAlive = false;
    glutPostRedisplay();

    //----------------------------------CHECK WIN CONDITION ----------------------//
    if(enemiesLeft <= 0 || (P->getPlayerLoc().x == M->GetChestLoc().x && P->getPlayerLoc().y == M->GetChestLoc().y ))
        isWin = true;
}

/* Program entry point */

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (800, 800);                //window screen
   glutInitWindowPosition (100, 100);            //window position
   glutCreateWindow ("Maze");                    //program title
   init();

   glutDisplayFunc(display);                     //callback function for display
   glutReshapeFunc(resize);                      //callback for reshape
   glutKeyboardFunc(key);                        //callback function for keyboard
   glutSpecialFunc(Specialkeys);
   glutMouseFunc(mouse);
   glutIdleFunc(idle);
   glutIdleFunc(updateStatus);
   glutMainLoop();

   return EXIT_SUCCESS;
}
