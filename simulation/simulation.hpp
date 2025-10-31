class Simulation {
private:
    Map map;
    std::vector<Creature> creatures;
    TurnManager turnManager;
    StatsTracker stats;
    
public:
    void init();
    void runTurn();
    void runNTurns(int n);
    
    const StatsTracker& getStats() const;
};