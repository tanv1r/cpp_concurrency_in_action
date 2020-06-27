#include <iostream>
#include <future>
#include <string>

using namespace std;

struct X
{
    void foo(int n, string const & msg)
    {
        cout << "n=" << n << " msg=" << msg << endl;
    }

    string bar(string const & msg)
    {
        string reply(msg);
        reply.append(": X");

        return reply;
    }
};

void baz(X & blah)
{
    cout << blah.bar("blah") << endl;
}

struct Y
{
    double operator() (double value)
    {
        return 2*value;
    }
};

class MoveOnly
{
    public:
        MoveOnly()
        {
        }

        MoveOnly(MoveOnly &&)
        {
        }

        MoveOnly(MoveOnly const &) = delete;
        MoveOnly & operator= (MoveOnly &&)
        {
        }

        MoveOnly & operator= (MoveOnly const &) = delete;

        void operator() ()
        {
            cout << "hi" << endl;
        }
};

int main()
{
    X x;

    // call x->foo() 
    auto f1 = async(&X::foo, &x, 3, "hello");

    // copy x and then call x.bar()
    auto f2 = async(&X::bar, x, "good bye!");

    f1.get();
    cout << f2.get() << endl;

    Y y;

    // create temporary Y and call ()
    auto f3 = async(Y(), 3.141);

    // call y's ()
    // if y had state, it would have been mutated
    //
    auto f4 = async(ref(y), 2.718);

    cout << f3.get() << endl;
    cout << f4.get() << endl;

    // call baz() with arg x
    auto f5 = async(baz, ref(x));

    f5.get();

    auto f6 = async(MoveOnly());

    // this is like C#'s Wait()
    f6.wait();

    return 0;
}