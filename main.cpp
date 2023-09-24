#include <iostream>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <thread>
#include <chrono>

const int SCREEN_WIDTH{800};
const int SCREEN_HEIGHT{600};
const int MAX_PARTICLES{15};
const int STEP_ADD_TIME{500};
const int STEP_PRINT_PARTICLES{3};

const double P_RADIUS{30}; // particle radius
const double S_RADIUS{2*P_RADIUS}; // smoothing radius
const double P_MASS{1};    // particle mass
const double ISOTROPIC_EXPONENT{20};
const double BASE_DENSITY{1};
const double DAMPING_COEFFICIENT{-0.6};
const double DYNAMIC_VISCOSITY{0.5};
const int TIME_STEP{10}; // milliseconds
sf::Vector2f GRAVITY_FORCE{0, 0.1};

const double COEFFICIENT_DENSITY{315 * P_MASS / (64 * M_PI * pow(S_RADIUS, 9))};
const double COEFFICIENT_PRESSURE_FORCE{-45 * P_MASS / (M_PI * pow(S_RADIUS, 6))};
const double COEFFICIENT_VISCOSITY{45 * P_MASS * DYNAMIC_VISCOSITY / (M_PI * pow(S_RADIUS, 6))};

double W(double r_i_j)
{
    if(r_i_j>(S_RADIUS)) return 0.0;
    return COEFFICIENT_DENSITY * pow((pow(S_RADIUS, 2) - pow(r_i_j, 2)), 3);
}

double getDistance(const sf::Vector2f &a, const sf::Vector2f &b)
{
    return sqrt(pow(abs(a.x - b.x), 2) + pow(abs(a.y - b.y), 2));
}

class Particle : public sf::CircleShape
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Vector2f force;
    double dens;
    double pres;
    double mass;
    int ID;

public:
    Particle(double &pos_x, double &pos_y, int &ID) : CircleShape(P_RADIUS), ID(ID)
    {
        pos.x = pos_x;
        pos.y = pos_y;

        vel.x = -0.5;
        vel.y = 0;

        force.x = 0;
        force.y = 0;

        dens = 0;
        pres = 0;

        mass = P_MASS;

        this->setPosition(pos.x, pos_y);
        this->setFillColor(sf::Color::Blue);
    }

    sf::Vector2f getPos()
    {
        return pos;
    }

    void setPos(sf::Vector2f &p)
    {
        pos = p;
    }

    sf::Vector2f getVel()
    {
        return vel;
    }

    void setVel(sf::Vector2f &v)
    {
        vel = v;
    }

    sf::Vector2f getForce()
    {
        return force;
    }

    void setForce(sf::Vector2f &f)
    {
        force = f;
    }

    double getDens()
    {
        return dens;
    }

    void setDens(double &d)
    {
        dens = d;
    }

    double getPres()
    {
        return pres;
    }

    void setPres(double &p)
    {
        pres = p;
    }

    int getID()
    {
        return ID;
    }
};

void SPH_algorithm()
{

    int time{0};
    int particle_n{0};

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "SFML Window");
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::vector<Particle> particles;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (time % STEP_ADD_TIME == 0 && (time / STEP_ADD_TIME) < MAX_PARTICLES)
        {
            for (int i = 0; i < STEP_PRINT_PARTICLES; i++)
            {
                double x = static_cast<double>(rand() % (STEP_PRINT_PARTICLES * static_cast<int>(P_RADIUS)) + 100);
                double y = static_cast<double>(rand() % static_cast<int>(P_RADIUS));

                particles.emplace_back(x, y, particle_n);
                particle_n++;
            }
        }

        for (auto &i : particles)
        {
            double density{0};

            for (auto j : particles)
            {
                if (i.getID() == j.getID())
                    continue;

                double r_i_j = getDistance(j.getPos(), i.getPos());

                density += P_MASS * W(r_i_j);
                std::cout<<W(r_i_j);
            }
            std::cout<<'\n';
            i.setDens(density);
            //std::cout << i.getDens() << "\n";
        }

        for (auto &i : particles)
        {
            double pressure{0};
            pressure = ISOTROPIC_EXPONENT * (i.getDens() - BASE_DENSITY);

            i.setPres(pressure);
            //std::cout << i.getPres() << "\n";
        }

        for (auto &i : particles)
        {
            sf::Vector2f force{0, 0};

            for (auto j : particles)
            {
                if (i.getID() == j.getID())
                    continue;

                double r_i_j = getDistance(j.getPos(), i.getPos());

                if(r_i_j==0) continue;

                force.x += -COEFFICIENT_PRESSURE_FORCE * (i.getPos().x - j.getPos().x) / r_i_j * (i.getPres() + j.getPres()) / (2 * j.getDens()) * pow((S_RADIUS - r_i_j), 2);
                force.y += -COEFFICIENT_PRESSURE_FORCE * (i.getPos().y - j.getPos().y) / r_i_j * (i.getPres() + j.getPres()) / (2 * j.getDens()) * pow((S_RADIUS - r_i_j), 2);

                force.x += COEFFICIENT_VISCOSITY * (j.getVel().x - i.getVel().x) / j.getDens() * (S_RADIUS - r_i_j);
                force.y += COEFFICIENT_VISCOSITY * (j.getVel().y - i.getVel().y) / j.getDens() * (S_RADIUS - r_i_j);
            }

            force += GRAVITY_FORCE;
            // std::cout<<force.x<<" "<<force.y<<" \n";

            i.setForce(force);
            //std::cout<<i.getForce().x<<" "<<i.getForce().y<<"\n";
        }

        for (auto &i : particles)
        {
            sf::Vector2f velocity{0, 0};
            sf::Vector2f position{0, 0};

            velocity.x +=  (TIME_STEP/1000.0) * i.getForce().x / P_MASS;
            velocity.y +=  (TIME_STEP/1000.0) * i.getForce().y / P_MASS;

            velocity += i.getVel();

            position.x +=  velocity.x;
            position.y +=  velocity.y;

            position += i.getPos();
            // std::cout<<i.getPos().x<<" "<<i.getPos().y<<" ";
            // std::cout<<i.getVel().x<<" "<<i.getVel().y<<" ";

            // BOUNDARY CONDITION
            /**/
            if (position.x > (SCREEN_WIDTH - 2 * S_RADIUS))
            {
                velocity.x *= DAMPING_COEFFICIENT;
                position.x = SCREEN_WIDTH - 2 * S_RADIUS;
            }
            if (position.x < S_RADIUS)
            {
                velocity.x *= DAMPING_COEFFICIENT;
                position.x = S_RADIUS;
            }
            if (position.y > (SCREEN_HEIGHT - S_RADIUS))
            {
                velocity.y *= DAMPING_COEFFICIENT;
                position.y = SCREEN_HEIGHT - S_RADIUS;
            }
            if (position.y < S_RADIUS)
            {
                velocity.y *= DAMPING_COEFFICIENT;
                position.y = S_RADIUS;
            }

            i.setVel(velocity);
            i.setPos(position);
            // std::cout<<velocity.x<<" "<<velocity.y<<" ";
        }

        window.clear();

        for (auto &i : particles)
        {
            i.setPosition(i.getPos());
            // std::cout << i.getPosition().x << " " << i.getPosition().y << "\n";
            std::cout<<i.getDens()<<" "<<i.getPres()<<" "<<i.getForce().x<<" "<<i.getForce().y<<" "<<i.getVel().x<<" "<<i.getVel().y<<'\n';
            window.draw(i);
        }

        window.display();

        

        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_STEP));
        time += TIME_STEP;
        std::cout << "\n============================\n";
    }
}

int main()
{
    SPH_algorithm();
}