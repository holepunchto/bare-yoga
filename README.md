# bare-yoga

[Yoga](https://github.com/facebook/yoga) bindings for Bare, providing flexible box layout independent of any platform UI toolkit.

```
npm i bare-yoga
```

## Usage

```js
const { Node, constants } = require('bare-yoga')

const root = new Node()
root.width = 300
root.height = 100
root.flexDirection = constants.FLEX_DIRECTION.ROW

const sidebar = new Node()
sidebar.width = 80

const content = new Node()
content.flexGrow = 1

root.insertChild(sidebar)
root.insertChild(content)

root.calculateLayout(300, 100)

console.log(content.layout) // { left: 80, top: 0, width: 220, height: 100 }
```

## License

Apache-2.0
