# Test

## Create game
```json
{"type":"add_game","data":{"type":"tic_tac_toe","data":{}}}
```

## Create state
```json
{"type":"add_state","data":{"gameID":1,"data":null}}
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
