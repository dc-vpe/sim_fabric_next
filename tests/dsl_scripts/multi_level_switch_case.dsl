//This script tests multi-level switch case statements.
var a;
var b;
var c;

a = 1; b = 3; c = 5; Run();
a = 1; b = 3; c = 6; Run();
a = 1; b = 4; c = 6; Run();
a = 2; b = 3; c = 5; Run();


var Run()
{
    print("\n");
    switch( a )
    {
        case 1:
        {
            print("1");
            switch( b )
            {
                case 3:
                {
                    print("3");
                    switch(c)
                    {
                        case 5:
                        {
                            print("5");
                            break;
                        }
                        case 6:
                        {
                            print("6");
                            break;
                        }
                    }
                }
                case 4:
                {
                    print("4");
                    break;
                }
            }
            break;
        }
        case 2:
        {
            print("2");
            break;
        }
    }
}

stop