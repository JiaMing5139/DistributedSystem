//
// Created by parallels on 4/29/20.
//

#ifndef MUDUO_STUDY_NONCOPYABLE_H
#define MUDUO_STUDY_NONCOPYABLE_H


class noncopyable {
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator = (const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;


};


#endif //MUDUO_STUDY_NONCOPYABLE_H
