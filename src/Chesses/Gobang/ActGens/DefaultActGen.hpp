#pragma once

namespace gobang
{
    template <typename Gobang>
    class DefaultActGen
    {
    public:
        inline static constexpr unsigned char Size = Gobang::Size;
        using Action = typename Gobang::Action;

    private:
        const Gobang *_Game;

    public:
        class ActionIterator
        {
            friend class DefaultActGen;

        private:
            const DefaultActGen *_ActGen = nullptr;
            Action _Action;
            inline explicit ActionIterator() = default;
            inline explicit ActionIterator(const DefaultActGen *actGen)
                : _ActGen(actGen) { ++*this; }

        public:
            inline bool operator!=(ActionIterator) const { return _Action.Row < Size; }
            inline Action operator*() const { return _Action; }
            void operator++()
            {
                while (true)
                {
                    ++_Action.Col;
                    if (_Action.Col == Size)
                    {
                        _Action.Col = 0;
                        ++_Action.Row;
                    }
                    if (_Action.Row >= Size)
                        break;
                    if (_ActGen->_Game->GetBoard()[_Action.Row][_Action.Col] == 2)
                        break;
                }
            }
        };

        inline explicit DefaultActGen(const Gobang &game) : _Game(&game) {}

        inline ActionIterator begin() const { return ActionIterator(this); }
        inline ActionIterator end() const { return ActionIterator(); }

        inline void SetGame(const Gobang &game) { _Game = &game; }
        inline void Notify(Action) const {}
    };
} // namespace gobang
