//
// Created by parallels on 5/12/20.
//

#ifndef MUDUO_STUDY_COPYABLE_H
#define MUDUO_STUDY_COPYABLE_H
class Copyable{
public:
    Copyable(const Copyable &) = default;
    Copyable &operator = (const Copyable &) = default;
protected:

    Copyable() = default;
    ~Copyable() = default;

};
#endif //MUDUO_STUDY_COPYABLE_H
