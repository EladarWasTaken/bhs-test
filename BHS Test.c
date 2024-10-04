using System;
using System.Collections.Generic;
using Leopotam.Ecs;

namespace Scene
{
    abstract class SceneObject
    {
        public abstract void UpdatePosition();
    }

    class Wall : SceneObject
    {
        public (float X1, float Y1, float X2, float Y2) Line;

        public Wall(float x1, float y1, float x2, float y2)
        {
            Line = (x1, y1, x2, y2);
        }

        public override void UpdatePosition() {}
    }

    class Ball : SceneObject
    {
        public float X;
        public float Y;
        public float SpeedX;
        public float SpeedY;

        public Ball(float x, float y, float speedX, float speedY)
        {
            X = x;
            Y = y;
            SpeedX = speedX;
            SpeedY = speedY;
        }

        public override void UpdatePosition()
        {
            X += SpeedX;
            Y += SpeedY;
        }
    }

    class Scene
    {
        public List<SceneObject> Objects = new List<SceneObject>();

        public Scene()
        {
            Objects.Add(new Wall(0, 0, 0, 100));
            Objects.Add(new Wall(0, 100, 100, 100));
            Objects.Add(new Wall(100, 100, 100, 0));
            Objects.Add(new Wall(100, 0, 0, 0));

            Objects.Add(new Ball(50, 50, 1, 1));
        }
    }

    struct Position
    {
        public float X;
        public float Y;
    }

    struct Velocity
    {
        public float SpeedX;
        public float SpeedY;
    }

    class MoveSystem : IEcsRunSystem
    {
        private EcsFilter<Position, Velocity> _filter;

        public void Run()
        {
            foreach (var i in _filter)
            {
                ref var position = ref _filter.Get(i).Item1;
                ref var velocity = ref _filter.Get(i).Item2;

                position.X += velocity.SpeedX;
                position.Y += velocity.SpeedY;
            }
        }
    }

    class BounceSystem : IEcsRunSystem
    {
        private EcsFilter<Position, Velocity> _filter;
        private List<Wall> _walls;

        public BounceSystem(List<Wall> walls)
        {
            _walls = walls;
        }

        public void Run()
        {
            foreach (var i in _filter)
            {
                ref var position = ref _filter.Get(i).Item1;
                ref var velocity = ref _filter.Get(i).Item2;

                foreach (var wall in _walls)
                {
                    if (position.X < wall.Line.X2 && position.X > wall.Line.X1 &&
                        position.Y < wall.Line.Y2 && position.Y > wall.Line.Y1)
                    {
                        velocity.SpeedX = -velocity.SpeedX;
                        velocity.SpeedY = -velocity.SpeedY;

                        Console.WriteLine($"Bounce off wall: ({wall.Line.X1}, {wall.Line.Y1}) to ({wall.Line.X2}, {wall.Line.Y2})");
                    }
                }
            }
        }
    }

    class Program
    {
        static void Main()
        {
            var scene = new Scene();
            var ecsWorld = new EcsWorld();
            var ecsSystems = new EcsSystems(ecsWorld);

            foreach (var obj in scene.Objects)
            {
                if (obj is Ball ball)
                {
                    var entity = ecsWorld.NewEntity();
                    entity.Get<Position>().X = ball.X;
                    entity.Get<Position>().Y = ball.Y;
                    entity.Get<Velocity>().SpeedX = ball.SpeedX;
                    entity.Get<Velocity>().SpeedY = ball.SpeedY;
                }
            }

            ecsSystems.Add(new MoveSystem());
            ecsSystems.Add(new BounceSystem(scene.Objects.OfType<Wall>().ToList()));
            ecsSystems.Init();

            while (true)
            {
                ecsSystems.Run();
                foreach (var i in ecsWorld.GetPool<Position>().GetAll())
                {
                    var pos = ecsWorld.GetPool<Position>()[i];
                    Console.WriteLine($"Ball position: ({pos.X}, {pos.Y})");
                }

                System.Threading.Thread.Sleep(100);
            }
        }
    }
}