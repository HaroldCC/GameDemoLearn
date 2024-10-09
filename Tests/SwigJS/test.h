

void TestHarold(const char *str1, const char *str2);

double TestHarold(double a, double b);

enum class StudentWork
{
    Normal,
    EnglistMaster,
    MathMaster,
};

enum TeachType
{
    Master,
    Chinese,
    Math,
};

class Student
{
public:
    void Eat();

    void Run();

    StudentWork GetWork();

private:
    const char *name;
    int         age;
    int         sex;
    StudentWork work;
};