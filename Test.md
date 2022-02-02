# Test

## Echo
```json
{"type":"echo","data":{"sleepTime":1.5,"data":"Hello world"}}
```

## Create game
```json
{"type":"add_game","data":{"type":"tic_tac_toe","data":{}}}
```

## Create default state
```json
{"type":"add_state","data":{"gameID":1}}
```

## Create state
```json
{"type":"add_state","data":{"gameID":1,"data":{"board":[[0,0,0],[0,1,0],[0,0,0]]}}}
```

## Create player
```json
{"type":"add_player","data":{"stateID":1,"type":"random_move","data":{"actionGenerator":{"type":"default","data":{}}}}}
```

## Create action generator
```json
{"type":"add_action_generator","data":{"stateID":1,"type":"default","data":{}}}
```

## Generate actions
```json
{"type":"generate_actions","data":{"actionGeneratorID":1}}
```

## Take action
```json
{"type":"take_action","data":{"stateID":1,"action":{"row":1,"col":1}}}
```

## Start thinking
```json
{"type":"start_thinking","data":{"playerID":1}}
```

## Stop thinking
```json
{"type":"stop_thinking","data":{"playerID":1}}
```

## Get best action
```json
{"type":"get_best_action","data":{"playerID":1,"maxThinkTime":1.5}}
```
