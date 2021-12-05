# Test

## Create game
```json
{"id":1,"type":"add_game","data":{"type":"tic_tac_toe","data":{}}}
```

## Create state
```json
{"id":2,"type":"add_state","data":{"gameID":1,"data":null}}
```

## Create action generator
```json
{"id":3,"type":"add_action_generator","data":{"stateID":1,"type":"default","data":{}}}
```

## Generate actions
```json
{"id":4,"type":"generate_actions","data":{"actionGeneratorID":1}}
```

## Take action
```json
{"id":5,"type":"take_action","data":{"stateID":1,"action":{"row":1,"col":1}}}
```
