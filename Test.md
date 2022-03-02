# Test

## echo
```json
{"type":"echo","data":{"sleepTime":1.5,"data":"Hello world"}}
```

## run_games
```json
{"type":"run_games","data":{"rounds":1,"parallel":false,"game":{"type":"tic_tac_toe","data":{}},"players":[{"type":"random_move","data":{"actionGenerator":{"type":"default","data":{}}},"maxThinkTime":1,"allowBackgroundThinking":true},{"type":"random_move","data":{"actionGenerator":{"type":"default","data":{}}},"maxThinkTime":1,"allowBackgroundThinking":true}]}}
{"type":"run_games","data":{"rounds":1,"parallel":false,"game":{"type":"gobang","data":{}},"players":[{"type":"mcts","data":{"iterations":1000,"explorationFactor":1,"goalMatrix":[[-1,1],[1,-1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}}},"allowBackgroundThinking":false},{"type":"mcts","data":{"iterations":1000,"explorationFactor":1,"goalMatrix":[[-1,1],[1,-1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}}},"allowBackgroundThinking":false}]}}
```

## add_game
```json
{"type":"add_game","data":{"type":"tic_tac_toe","data":{}}}
```

## add_state
```json
{"type":"add_state","data":{"gameID":1}}
{"type":"add_state","data":{"gameID":1,"data":{"board":[[0,0,0],[0,1,0],[0,0,0]]}}}
```

## add_player
```json
{"type":"add_player","data":{"gameID":1,"stateID":1,"type":"random_move","data":{"actionGenerator":{"type":"default","data":{}}}}}
```

## add_action_generator
```json
{"type":"add_action_generator","data":{"gameID":1,"stateID":1,"type":"default","data":{}}}
```

## remove_game
```json
{"type":"remove_game","data":{"gameID":1}}
```

## remove_state
```json
{"type":"remove_state","data":{"gameID":1,"stateID":1}}
```

## remove_player
```json
{"type":"remove_player","data":{"gameID":1,"stateID":1,"playerID":1}}
```

## remove_action_generator
```json
{"type":"remove_action_generator","data":{"gameID":1,"stateID":1,"actionGeneratorID":1}}
```

## generate_actions
```json
{"type":"generate_actions","data":{"gameID":1,"stateID":1,"actionGeneratorID":1}}
```

## take_action
```json
{"type":"take_action","data":{"gameID":1,"stateID":1,"action":{"row":1,"col":1}}}
```

## start_thinking
```json
{"type":"start_thinking","data":{"gameID":1,"stateID":1,"playerID":1}}
```

## stop_thinking
```json
{"type":"stop_thinking","data":{"gameID":1,"stateID":1,"playerID":1}}
```

## get_best_action
```json
{"type":"get_best_action","data":{"gameID":1,"stateID":1,"playerID":1,"maxThinkTime":1.5}}
```
