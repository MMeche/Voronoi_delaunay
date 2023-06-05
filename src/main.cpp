#include "application_ui.h"
#include "SDL2_gfxPrimitives.h"
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <algorithm>


#define EPSILON 0.0001f

struct Coords
{
    int x, y;

    bool operator==(const Coords& other)
    {
        return x == other.x and y == other.y;
    }
};

struct Segment
{
    Coords p1, p2;
};

struct Triangle
{
    Coords p1, p2, p3;
    bool complet=false;
};

struct CentreVoronoi
{
    Coords c;
    Triangle t;
};

struct SegmentVoronoi
{
    Coords c1;
    Coords c2;
};

struct Application
{
    int width, height;
    Coords focus{100, 100};

    std::vector<Coords> points;
    std::vector<Triangle> triangles;
    std::vector<CentreVoronoi> centres;
    std::vector<SegmentVoronoi> segments;

};

bool compareCoords(Coords point1, Coords point2)
{
    if (point1.x == point2.x)
        return point1.y < point2.y;
    return point1.x < point2.x;
}

void drawPoints(SDL_Renderer *renderer,const Application &app)
{
    for (std::size_t i = 0; i < app.points.size(); i++)
    {
        filledCircleRGBA(renderer, app.points[i].x, app.points[i].y, 3, 240, 240, 23, SDL_ALPHA_OPAQUE);
    }
}

void drawVoronoi(SDL_Renderer *renderer, const Application &app)
{
    for(std::size_t i = 0 ; i < app.centres.size();i++)
    {
        filledCircleRGBA(renderer, app.centres[i].c.x, app.centres[i].c.y,3,240,240,0,SDL_ALPHA_OPAQUE);
    };
    for(std::size_t i = 0; i < app.segments.size();i++)
    {
        lineRGBA(
            renderer,
            app.segments[i].c1.x, app.segments[i].c1.y,
            app.segments[i].c2.x, app.segments[i].c2.y,
            240,240,0,SDL_ALPHA_OPAQUE);
    }
};

void drawSegments(SDL_Renderer *renderer, const std::vector<Segment> &segments)
{
    for (std::size_t i = 0; i < segments.size(); i++)
    {
        lineRGBA(
            renderer,
            segments[i].p1.x, segments[i].p1.y,
            segments[i].p2.x, segments[i].p2.y,
            240, 240, 20, SDL_ALPHA_OPAQUE);
    }
}

void drawTriangles(SDL_Renderer *renderer, const std::vector<Triangle> &triangles)
{
    for (std::size_t i = 0; i < triangles.size(); i++)
    {
        const Triangle& t = triangles[i];
        trigonRGBA(
            renderer,
            t.p1.x, t.p1.y,
            t.p2.x, t.p2.y,
            t.p3.x, t.p3.y,
            0, 240, 160, SDL_ALPHA_OPAQUE
        );


    }
}

void draw(SDL_Renderer *renderer, const Application &app)
{
    /* Remplissez cette fonction pour faire l'affichage du jeu */
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    drawPoints(renderer, app);
    /*drawTriangles(renderer, app.triangles);*/
    drawVoronoi(renderer,app);
}

/*
   Détermine si un point se trouve dans un cercle définit par trois points
   Retourne, par les paramètres, le centre et le rayon
*/
bool CircumCircle(
    float pX, float pY,
    float x1, float y1, float x2, float y2, float x3, float y3,
    float *xc, float *yc, float *rsqr
)
{
    float m1, m2, mx1, mx2, my1, my2;
    float dx, dy, drsqr;
    float fabsy1y2 = fabs(y1 - y2);
    float fabsy2y3 = fabs(y2 - y3);
    
    /* Check for coincident points */
    if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
        return (false);

    if (fabsy1y2 < EPSILON)
    {
        m2 = -(x3 - x2) / (y3 - y2);
        mx2 = (x2 + x3) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (x2 + x1) / 2.0;
        *yc = m2 * (*xc - mx2) + my2;
    }
    else if (fabsy2y3 < EPSILON)
    {
        m1 = -(x2 - x1) / (y2 - y1);
        mx1 = (x1 + x2) / 2.0;
        my1 = (y1 + y2) / 2.0;
        *xc = (x3 + x2) / 2.0;
        *yc = m1 * (*xc - mx1) + my1;
    }
    else
    {
        
        m1 = -(x2 - x1) / (y2 - y1);
        m2 = -(x3 - x2) / (y3 - y2);
        mx1 = (x1 + x2) / 2.0;
        mx2 = (x2 + x3) / 2.0;
        my1 = (y1 + y2) / 2.0;
        my2 = (y2 + y3) / 2.0;
        *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
        if (fabsy1y2 > fabsy2y3)
        {
            
            *yc = m1 * (*xc - mx1) + my1;
        }
        else
        {
            *yc = m2 * (*xc - mx2) + my2;
        }
    }

    dx = x2 - *xc;
    dy = y2 - *yc;
    *rsqr = dx * dx + dy * dy;

    dx = pX - *xc;
    dy = pY - *yc;
    drsqr = dx * dx + dy * dy;
    
    return ((drsqr - *rsqr) <= EPSILON ? true : false);
}

void construitDelaunay(Application &app)
{
    std::sort(app.points.begin(), app.points.end(), compareCoords);
    app.triangles.clear();
    
    

    Coords p1{-1000,-1000};
    Coords p2{500,3000};
    Coords p3{1500,-1000};
    app.triangles.push_back(Triangle{p1,p2,p3,false});
    

    /*Pour chaque points placés*/
    for(std::size_t i = 0 ; i < app.points.size() ; i++)
    {
        Coords P = app.points[i];
        std::vector<Segment> LS;

        /*Pour chaque triangle déjà créé*/
        for(std::size_t j = 0 ; j < app.triangles.size() ; j++)
        {
            Triangle T = app.triangles[j];
            float xc,yc,rsqr;
            
            /* Test pour savoir si le point est dans le cercle circonscrit */
            if(CircumCircle(
                P.x,P.y,
                T.p1.x,T.p1.y,
                T.p2.x,T.p2.y,
                T.p3.x,T.p3.y,
                &xc,&yc,&rsqr))
            {
                /*Si oui, on remplit LS des segments du triangle*/
                LS.push_back(Segment{T.p1,T.p2});
                LS.push_back(Segment{T.p2,T.p3});
                LS.push_back(Segment{T.p3,T.p1});
                app.triangles.erase(app.triangles.begin()+j);
                /*app.centres.erase(app.centres.begin()+j);*/
                /*Et on enlève ledit triangle de la liste des triangles*/
                j--; /* Si on supprime le triangle, on reste sur place dans la liste */
            }
        };
        
        /*Pour chaque segment S de LS*/
        for(std::size_t j = 0 ; j < LS.size() ; j++)
        {
            Segment S = LS[j];
            bool doublon = false; 
            /*On va tester si il y a des doublons, si oui on enlevera le segment test à la fin*/
            for(std::size_t k = j+1 ; k < LS.size() ; k++)
            {   /*On fait la liste de tous les indices des segments redondants*/
                if((S.p1 == LS[k].p1)&&(S.p2 == LS[k].p2)||(S.p1 == LS[k].p2)&&(S.p2 == LS[k].p1))
                {
                    LS.erase(LS.begin()+k);
                    doublon = true;
                    k--;
                };
            };
            if(doublon)
            {
                LS.erase(LS.begin()+j);
                j--;
            };
        };

        for(std::size_t j = 0 ; j < LS.size() ; j++)
        {
            app.triangles.push_back(Triangle{LS[j].p1,LS[j].p2,P});            
        };
    };
    
}

void construitVoronoi(Application &app)
{
    app.centres.clear();
    app.segments.clear();
    for(std::size_t i = 0 ; i < app.triangles.size();i++)
    {
        float xc,yc,rsqr;
        CircumCircle(app.triangles[i].p1.x,app.triangles[i].p1.y,
                    app.triangles[i].p1.x,app.triangles[i].p1.y,
                    app.triangles[i].p2.x,app.triangles[i].p2.y,
                    app.triangles[i].p3.x,app.triangles[i].p3.y,
                    &xc,&yc,&rsqr);
        CentreVoronoi new_centre;
        new_centre.c.x = xc;
        new_centre.c.y = yc;
        new_centre.t = app.triangles[i];
        app.centres.push_back(new_centre);
    }
    for(std::size_t i = 0 ; i < app.centres.size() ; i++)
    {
        for(std::size_t j = 0 ; j < app.centres.size() ; j++)
        {
            Triangle triangle1 = app.centres[i].t;
            Triangle triangle2 = app.centres[j].t; 
            if(j!=i && 
            ((triangle1.p1 == triangle2.p1)&&(triangle1.p2 == triangle2.p2)||(triangle1.p1 == triangle2.p2)&&(triangle1.p2 == triangle2.p1))
            ||((triangle1.p1 == triangle2.p2)&&(triangle1.p2 == triangle2.p3)||(triangle1.p1 == triangle2.p3)&&(triangle1.p2 == triangle2.p2))
            ||((triangle1.p1 == triangle2.p3)&&(triangle1.p2 == triangle2.p1)||(triangle1.p1 == triangle2.p1)&&(triangle1.p2 == triangle2.p3))
            
            ||((triangle1.p2 == triangle2.p2)&&(triangle1.p3 == triangle2.p3)||(triangle1.p2 == triangle2.p3)&&(triangle1.p3 == triangle2.p2))
            ||((triangle1.p2 == triangle2.p2)&&(triangle1.p3 == triangle2.p3)||(triangle1.p2 == triangle2.p3)&&(triangle1.p3 == triangle2.p2))
            ||((triangle1.p2 == triangle2.p3)&&(triangle1.p3 == triangle2.p1)||(triangle1.p2 == triangle2.p1)&&(triangle1.p3 == triangle2.p3))
            
            ||((triangle1.p3 == triangle2.p2)&&(triangle1.p1 == triangle2.p3)||(triangle1.p3 == triangle2.p3)&&(triangle1.p1 == triangle2.p2))
            ||((triangle1.p3 == triangle2.p2)&&(triangle1.p1 == triangle2.p3)||(triangle1.p3 == triangle2.p3)&&(triangle1.p1 == triangle2.p2))
            ||((triangle1.p3 == triangle2.p3)&&(triangle1.p1 == triangle2.p1)||(triangle1.p3 == triangle2.p1)&&(triangle1.p1 == triangle2.p3)))
            {
                app.segments.push_back(SegmentVoronoi{app.centres[i].c,app.centres[j].c});
            }
        }
    }
};

bool handleEvent(Application &app,SDL_Renderer *renderer, const std::vector<Coords> &points)
{
    /* Remplissez cette fonction pour gérer les inputs utilisateurs */
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return false;
        else if (e.type == SDL_WINDOWEVENT_RESIZED)
        {
            app.width = e.window.data1;
            app.height = e.window.data1;
        }
        else if (e.type == SDL_MOUSEWHEEL)
        {
        }
        else if (e.type == SDL_MOUSEBUTTONUP)
        {
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                app.focus.x = e.button.x;
                app.focus.y = e.button.y;
                app.points.clear();
                app.centres.clear();
                app.triangles.clear();
            }
            else if (e.button.button == SDL_BUTTON_LEFT)
            {
                app.focus.y = 0;
                bool point_deja_pose=false;
                for (std::size_t i = 0; i < points.size(); i++)
                {
                    if(points[i].x==e.button.x && points[i].y==e.button.y)
                    {
                        point_deja_pose=true;
                    }
                }
                if(not point_deja_pose)
                { 
                    app.points.push_back(Coords{e.button.x, e.button.y});
                    construitDelaunay(app);
                    construitVoronoi(app);
                }
                /*if(app.points.size() > 2)
                {
                    int n = app.points.size();
                    app.triangles.push_back(Triangle{app.points[n-3],app.points[n-2],app.points[n-1]});
                } Milestone 0*/
                

                
            }
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    SDL_Window *gWindow;
    SDL_Renderer *renderer;
    Application app{720, 720, Coords{0, 0}};
    bool is_running = true;

    // Creation de la fenetre
    gWindow = init("Awesome Voronoi", 720, 720);

    if (!gWindow)
    {
        SDL_Log("Failed to initialize!\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(gWindow, -1, 0); // SDL_RENDERER_PRESENTVSYNC

    /*  GAME LOOP  */
    while (true)
    {
        // INPUTS
        is_running = handleEvent(app,renderer,app.points);
        if (!is_running)
            break;

        // EFFACAGE FRAME
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // DESSIN
        draw(renderer, app);

        // VALIDATION FRAME
        SDL_RenderPresent(renderer);

        // PAUSE en ms
        SDL_Delay(1000 / 30);
    }

    // Free resources and close SDL
    close(gWindow, renderer);

    return 0;
}

