# Computer Animation and Simulation Topics

These projects were implemented in the Standalone Interactive Graphics (SIG) Toolkit, created by Professor Marcelo Kallmann, and Unity.

Note: In order to run these projects, the files must be dropped in the SIG folder. Compile & Run on VS 2017. For more information, see the SIG examples/sample code [here](https://bitbucket.org/mkallmann/sig)

1. Particle Systems
   - Simulated a net with many particles that produces an interesting physics-based collision with falling balls.
   - Spring forces maintain the net particle-based structure
   - Collision detection and reaction is handled with spring reaction forces.
   - Euler and Verlet integration solvers are used.
   - The report/benchmarks for this project can be found here: [Particle Systems Report](https://github.com/AmarSaini/Computer-Simulation-and-Animation/blob/master/Particle%20Systems/Particle%20Systems%20Report.pdf)
   
![Particle Systems Preview](https://raw.githubusercontent.com/AmarSaini/Computer-Simulation-and-Animation/master/Particle%20Systems/preview.PNG)
   
2. Move Graphs
   - A skeleton with a hierarchy of many joints performs animations via move graphs.
   - Global root rotations are incremented to slightly deform the straight walk animation to the left or to the right, in order to create a turning animation without the need to capture the motion.
   - The report/benchmarks for this project can be found here: [Move Graphs Report](https://github.com/AmarSaini/Computer-Simulation-and-Animation/blob/master/Move%20Graphs/Move%20Graphs%20Report.pdf)
   
![Move Graphs Preview](https://raw.githubusercontent.com/AmarSaini/Computer-Simulation-and-Animation/master/Move%20Graphs/preview.PNG)
   
3. Steering
   - Simulated many agents (cylinders) to navigate with reactive behaviors to random targets.
   - Steering behaviors, such as: Seek, Flee, Arrival, Wander, Pursue, Evade, and Follow the Leader were implemented.
   - The report/benchmarks for this project can be found here: [Steering Report](https://github.com/AmarSaini/Computer-Simulation-and-Animation/blob/master/Steering/Steering%20Report.pdf)
   
![Steering Preview](https://raw.githubusercontent.com/AmarSaini/Computer-Simulation-and-Animation/master/Steering/preview.PNG)
