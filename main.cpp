#include <iostream>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <thread>
#include <chrono>
#include <eigen3/Eigen/Dense>

const int SCREEN_WIDTH{800};
const int SCREEN_HEIGHT{600};
const int MAX_PARTICLES{150};
const int STEP_ADD_TIME{500};
const int STEP_PRINT_PARTICLES{3};

const double P_RADIUS{4};    // particle radius
const double S_RADIUS{11.0}; // smoothing radius
const double P_MASS{1.5};    // particle mass
const double ISOTROPIC_EXPONENT{50.0};
const double BASE_DENSITY{4.0};
const double DAMPING_COEFFICIENT{-0.5};
const double DYNAMIC_VISCOSITY{25.0};
const int TIME_STEP{5}; // milliseconds
Eigen::Vector2d GRAVITY_FORCE{0, 1000.0};

const double COEFFICIENT_DENSITY{315.0 / (64.0 * M_PI * pow(S_RADIUS, 9))};
const double COEFFICIENT_PRESSURE_FORCE{-45.0 / (M_PI * pow(S_RADIUS, 6))};
const double COEFFICIENT_VISCOSITY{45.0 * DYNAMIC_VISCOSITY / (M_PI * pow(S_RADIUS, 6))};



double W(double r_i_j)
{
    return COEFFICIENT_DENSITY * pow((pow(S_RADIUS, 2) - pow(r_i_j, 2)), 3);
}

double grad_W(double r_i_j)
{
    return COEFFICIENT_PRESSURE_FORCE * pow((S_RADIUS - r_i_j), 3);
}

double visc_W(double r_i_j)
{
    return COEFFICIENT_VISCOSITY * (S_RADIUS - r_i_j);
}



class Particle : public sf::CircleShape
{
    Eigen::Vector2d pos, vel, force;
    double dens, pres, mass;
    int ID;

public:
    Particle(double &pos_x, double &pos_y, int &ID) : CircleShape(P_RADIUS), ID(ID)
    {
        pos(0) = pos_x;
        pos(1) = pos_y;

        vel(0) = 0;
        vel(1) = 0;

        force(0) = 0;
        force(1) = 0;

        dens = 0;
        pres = 0;

        mass = P_MASS;

        this->setPosition(pos(0), pos(1));
        this->setFillColor(sf::Color::Blue);
    }

    Eigen::Vector2d getPos()
    {
        return pos;
    }

    void setPos(Eigen::Vector2d &p)
    {
        pos = p;
    }

    Eigen::Vector2d getVel()
    {
        return vel;
    }

    void setVel(Eigen::Vector2d &v)
    {
        vel = v;
    }

    Eigen::Vector2d getForce()
    {
        return force;
    }

    void setForce(Eigen::Vector2d &f)
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

    for (double i = 200; i < 400; i += 9)
    {
        for (double j = 500; j < 580; j += 9)
        {
            particles.emplace_back(i, j, ++particle_n);
        }
    }

    for (auto &i : particles)
    {
        i.setPosition(i.getPos()(0), i.getPos()(1));
        window.draw(i);
    }

    window.display();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        /*if (time % STEP_ADD_TIME == 0 && (time / STEP_ADD_TIME) < MAX_PARTICLES)
        {
            for (int i = 0; i < STEP_PRINT_PARTICLES; i++)
            {
                double x = static_cast<double>(rand() % (STEP_PRINT_PARTICLES * 2* static_cast<int>(P_RADIUS)) + 100);
                double y = static_cast<double>(rand() % static_cast<int>(P_RADIUS));

                particles.emplace_back(x, y, particle_n);
                particle_n++;
            }
        }*/

        for (auto &i : particles)
        {
            double density{0};

            for (auto &j : particles)
            {
                if (i.getID() == j.getID())
                    continue;

                Eigen::Vector2d r_i_j{j.getPos() - i.getPos()};
                double r = r_i_j.norm();

                if (r > S_RADIUS)
                    continue;

                density += P_MASS * W(r);
            }
            std::cout << '\n';
            if (density == 0.0)
                density = 0.03;
            i.setDens(density);
            // std::cout << i.getDens() << "\n";
        }

        for (auto &i : particles)
        {
            double pressure{0};
            pressure = ISOTROPIC_EXPONENT * (i.getDens() - BASE_DENSITY);

            i.setPres(pressure);
            // std::cout << i.getPres() << "\n";
        }

        for (auto &i : particles)
        {
            Eigen::Vector2d fpress{0.0, 0.0};
            Eigen::Vector2d fvisc{0.0, 0.0};
            Eigen::Vector2d fgrav{GRAVITY_FORCE };

            for (auto &j : particles)
            {
                if (i.getID() == j.getID())
                    continue;

                Eigen::Vector2d r_i_j{j.getPos() - i.getPos()};
                double r = r_i_j.norm();

                if (r > S_RADIUS)
                    continue;

                fpress += -r_i_j.normalized() * P_MASS * grad_W(r) * (i.getPres() + j.getPres()) / (2*i.getDens());

                fvisc += P_MASS * visc_W(r) * (j.getVel() - i.getVel()) / (i.getDens());
            }

            // std::cout<<force.x<<" "<<force.y<<" \n";
            Eigen::Vector2d force = fpress + fvisc + fgrav;

            i.setForce(force);
            // std::cout<<i.getForce().x<<" "<<i.getForce().y<<"\n";
        }

        for (auto &i : particles)
        {
            Eigen::Vector2d velocity{0, 0};
            Eigen::Vector2d position{0, 0};

            velocity += (TIME_STEP / 1000.0) * i.getForce() / P_MASS;

            velocity += i.getVel();

            position += (TIME_STEP / 1000.0) * velocity;

            position += i.getPos();
            // std::cout<<i.getPos().x<<" "<<i.getPos().y<<" ";
            // std::cout<<i.getVel().x<<" "<<i.getVel().y<<" ";

            // BOUNDARY CONDITION
            if (position(0) > (SCREEN_WIDTH - S_RADIUS))
            {
                velocity(0) *= DAMPING_COEFFICIENT;
                position(0) = SCREEN_WIDTH - S_RADIUS;
            }
            if (position(0) < S_RADIUS)
            {
                velocity(0) *= DAMPING_COEFFICIENT;
                position(0) = S_RADIUS;
            }
            if (position(1) > (SCREEN_HEIGHT - S_RADIUS))
            {
                velocity(1) *= DAMPING_COEFFICIENT;
                position(1) = SCREEN_HEIGHT - S_RADIUS;
            }
            if (position(1) < S_RADIUS)
            {
                velocity(1) *= DAMPING_COEFFICIENT;
                position(1) = S_RADIUS;
            }

            i.setVel(velocity);
            i.setPos(position);
            // std::cout<<velocity.x<<" "<<velocity.y<<" ";
        }

        window.clear();

        for (auto &i : particles)
        {
            i.setPosition(i.getPos()(0), i.getPos()(1));
            // std::cout << i.getPosition().x << " " << i.getPosition().y << "\n";
            std::cout << "Dens: " << i.getDens() << " Pres: " << i.getPres() << " Force X: " << i.getForce()(0) << " Force Y: " << i.getForce()(1) << " Vel X: " << i.getVel()(0) << " Vel Y: " << i.getVel()(1) << " Pos X: " << i.getPos()(0) << " Pos Y: " << i.getPos()(1) << '\n';
            window.draw(i);
        }

        window.display();

        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_STEP));
        time += TIME_STEP;
        // std::cout << "\n============================\n";
    }
}

int main()
{
    SPH_algorithm();
}