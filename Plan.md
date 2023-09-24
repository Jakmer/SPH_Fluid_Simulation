## 1. Initialize Particles

Fluid will be represented by set of discrete particles. Each of them will properties like:

- Position
- Velocity
- Mass
- Density
- Pressure

## 2. Density Calculation

For each particle density ( ρ ) will be a sum of contributions of neighbours. To operate the sensivity depeneded on distance beetwen the current particle and every neighbour I will use **SMOOTHING KERNEL FUNCTION** assigned as **W(r_i_j)**

    W(r_i_j) = ( 315 * (h^2 - r^2)^3 ) / ( 64 * PI * h^9 )

Finaly

    ρ_i = Σ_j  m_j * W(r-i_j)

Where r_i_j is a distance between particles and h is a smoothing radius and m_j is mass

## 3. Pressure Calculation

To calculate pressure for each particle I will use an Equation Of State

    P = ρ * R * T

Where P is pressure, ρ is density, R is Gas Constant, T is temperature in Kelvin

## 4. Force Computation

To calculate force acting of each particle I need to include pressure force and external force (in this case Gravity). The force is crucial aspect for the particle motion. To calculate pressure force I will use **GRADIENT OF SMOOTHING KERNEL FUNCTION** assigned as **▽W(r_i_j)**

    ▽W(r_i_j) = ( - 45 * (h - r)^2 ) / ( PI * h^6 )

Finally

    F_p = - Σ_j ( m_j * ( P_i + P_j ) * ▽W(r^_i_j) ) / ( 2 * ρ_i * ρ_j )
    F_w = F_p + F_g

Where ρ - density, F_g = gravity force, P - pressure, m - mass, F_w - total force, F_p - pressure force

## 5. Integration with Timestep

Update the positions and velocities of particles based on the forces computed in the previous step using timestep Δt.

- Update velocities

  u_i = u_i + Δt * F_w / m_i

- Update positions

  x_i = x_i + Δt * u_i

## 6. Boundary Condition

Check whether the particles crossed the solid object like wall, etc.
The condition is about: when particle position ∈ boundary then reverse some compontents of particles's velocity

## 7. Visualization
Using SMFL present each particle as a blue circle

## 8. Iteration
