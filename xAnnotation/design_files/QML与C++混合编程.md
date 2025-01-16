https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html

QML 旨在通过 C++ 代码轻松扩展。 Qt Qml 模块中的类使 QML 对象能够从 C++ 加载和操作，并且 QML 引擎与 Qt 的 元对象系统 的集成性质使得可以直接从 QML 调用 C++ 功能。这允许开发通过 QML、JavaScript 和 C++ 代码的混合实现的应用程序。

QML 和 C++ 的集成提供了多种机会，包括以下能力：

- 将用户界面代码与应用程序逻辑代码分离，前者使用 QML 和 JavaScript 在 QML 文档 中实现，后者使用 C++ 实现
- 从 QML 使用和调用一些 C++ 功能（例如，调用您的应用程序逻辑，使用用 C++ 实现的数据模型，或调用第三方 C++ 库中的一些函数）
- 访问 Qt Qml 或 Qt Quick C++ API 的功能（例如，使用 QQuickImageProvider 动态生成图像）
- 从 C++ 实现您自己的 QML 对象类型 —— 无论是用于您自己的特定应用程序，还是分发给其他人

要将一些 C++ 数据或功能提供给 QML，必须从 QObject 派生的类中提供。由于 QML 引擎与元对象系统的集成，任何 QObject 派生类的属性、方法和信号都可以从 QML 访问，如在“将 C++ 类型的属性暴露给 QML”小节中所述。一旦此类提供了所需的功能，它可以通过多种方式暴露给 QML：

- 该类可以注册为可实例化的 QML 类型，以便可以从 QML 代码中实例化并像任何普通的 QML 对象类型一样使用
- 该类可以注册为单例类型，以便可以从 QML 代码中导入该类的单个实例，从而可以从 QML 访问该实例的属性、方法和信号
- 该类的实例可以作为上下文属性或上下文对象嵌入到 QML 代码中，从而可以从 QML 访问该实例的属性、方法和信号

这些是从 QML 代码访问 C++ 功能的最常见方法；有关更多选项和详细信息，请参阅下面各部分中描述的主要文档页面。此外，除了能够从 QML 访问 C++ 功能外，Qt Qml 模块还提供了反向操作的方法，可以从 C++ 代码操作 QML 对象。有关详细信息，请参阅“从 C++ 与 QML 对象交互https://doc.qt.io/qt-6/qtqml-cppintegration-interactqmlfromcpp.html”。

通常希望将某些状态作为全局属性暴露给 QML。“从 C++ 向 QML 暴露状态”小节描述了如何做到这一点。

最后，C++ 代码可以集成到 C++ 应用程序或 C++ 插件中，具体取决于它是作为独立应用程序还是库分发。插件可以与 QML 模块集成，然后可以由其他应用程序中的 QML 代码导入和使用；有关更多信息，请参阅“在 C++ 插件中提供类型和功能”。

# 在C++和QML之间选择正确的集成方式

为了快速地确定哪种集成方式适合你当前的情形，可以使用下面的流图：

![cpp-qml-integration-flowchart](D:\workspace\DriverPanel\design_files\QML与C++混合编程.assets\cpp-qml-integration-flowchart.png)

# 暴露C++类的属性到QML

https://doc.qt.io/qt-6/qtqml-cppintegration-exposecppattributes.html

由于 QML 引擎与 Qt 元对象系统的集成，QML 可以轻松地从 C++ 扩展。这种集成允许任何 QObject 派生类的属性、方法和信号从 QML 访问：属性可以读取和修改，方法可以从 JavaScript 表达式调用，并且根据需要自动为信号创建信号处理程序。此外，QObject 派生类的枚举值也可以从 QML 访问。这使得 C++ 数据和函数可以直接从 QML 访问，通常只需很少或不需要修改。

QML 引擎能够通过元对象系统自省 QObject 实例。这意味着任何 QML 代码都可以访问 QObject 派生类实例的以下成员：

- 属性
- 方法（前提是它们是公共槽或用 Q_INVOKABLE 标记）
- **信号**

此外，如果枚举已使用 Q_ENUM 声明，它们也可以使用。有关详细信息，请参阅“QML 和 C++ 之间的数据类型转换https://doc.qt.io/qt-6/qtqml-cppintegration-data.html”

一般来说，这些内容可以从 QML 访问，无论 QObject 派生类是否已在 QML 类型系统中注册。然而，如果类需要以某种方式使用，要求引擎访问额外的类型信息——例如，如果类本身要用作方法参数或属性，或者其枚举类型之一要以这种方式使用——那么可能需要注册。建议对所有在 QML 中使用的类型进行注册，因为只有注册的类型才能在编译时进行分析。

Q_GADGET 类型需要注册，因为它们不派生自已知的公共基类，无法自动提供。没有注册，它们的属性和方法是无法访问的。

您可以通过在 `qt_add_qml_module` 调用中使用 `DEPENDENCIES` 选项添加依赖项，使来自不同模块的 C++ 类型在您自己的模块中可用。例如，您可能希望依赖 QtQuick，以便您的 QML 暴露的 C++ 类型可以使用 `QColor` 作为方法参数和返回值。QtQuick 将 `QColor` 暴露为值类型 `color`。此类依赖项可能会在运行时自动推断，但您不应依赖于此。

还请注意，本文件中涵盖的许多重要概念在“**使用 C++ 编写 QML 扩展**”教程中进行了演示。

有关 C++ 和不同 QML 集成方法的更多信息，请参阅 **C++ 和 QML 集成概述**页面。

## 数据类型处理和所有权

从 C++ 传输到 QML 的任何数据，无论是作为属性值、方法参数或返回值，还是信号参数值，都必须是 QML 引擎支持的类型。

默认情况下，引擎支持许多 Qt C++ 类型，并且可以在从 QML 使用时自动转换它们。此外，注册到 QML 类型系统的 C++ 类可以用作数据类型，如果适当注册，它们的枚举也可以用作数据类型。有关更多信息，请参阅“QML 和 C++ 之间的数据类型转换”。

此外，当数据从 C++ 传输到 QML 时，会考虑数据所有权规则。有关更多详细信息，请参阅“数据所有权”。

## 暴露属性

可以使用 Q_PROPERTY() 宏为任何 QObject 派生类指定属性。属性是具有相关读取函数和可选写入函数的类数据成员。

所有 QObject 派生类或 Q_GADGET 类的属性都可以从 QML 访问。

例如，下面是一个具有 author 属性的 Message 类。根据 Q_PROPERTY 宏调用指定，此属性可以通过 author() 方法读取，并通过 setAuthor() 方法写入：

```c++
class Message : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
public:
    void setAuthor(const QString &a)
    {
        if (a != m_author) {
            m_author = a;
            emit authorChanged();
        }
    }

    QString author() const
    {
        return m_author;
    }

signals:
    void authorChanged();

private:
    QString m_author;
};
```

为了使得Message可用，你需要在C++类中使用QML_ELEMENT，以及在CMake中使用**qt_add_qml_module**。

可以将 Message 的实例作为所需属性传递给名为 MyItem.qml 的文件，以使其可用：

```c++
int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQuickView view;
    Message msg;
    view.setInitialProperties({{"msg", &msg}});
    view.setSource(QUrl::fromLocalFile("MyItem.qml"));
    view.show();

    return app.exec();
}
```

然后，可以从 MyItem.qml 读取 author 属性：

```qml
// MyItem.qml
import QtQuick

Text {
    required property Message msg

    width: 100; height: 100
    text: msg.author    // invokes Message::author() to get this value

    Component.onCompleted: {
        msg.author = "Jonah"  // invokes Message::setAuthor()
    }
}
```

为了最大限度地与 QML 互操作，任何可写属性都应具有一个关联的 NOTIFY 信号，该信号在属性值更改时发出。这允许属性与属性绑定一起使用，这是 QML 的一个基本特性，通过在任何依赖项的值更改时自动更新属性来强制执行属性之间的关系。

在上面的示例中，author 属性的关联 NOTIFY 信号是 authorChanged，如 Q_PROPERTY() 宏调用中所指定的。这意味着每当信号发出时——例如在 Message::setAuthor() 中 author 更改时——这会通知 QML 引擎，任何涉及 author 属性的绑定都必须更新，进而引擎将通过再次调用 Message::author() 来更新 text 属性。

如果 author 属性是可写的但没有关联的 NOTIFY 信号，则 text 值将使用 Message::author() 返回的初始值进行初始化，但不会随着该属性的任何后续更改而更新。此外，任何尝试从 QML 绑定到该属性的操作都会在运行时产生引擎警告。

### 使用Notify信号的注意事项

为了防止循环或过度评估，开发人员应确保仅在属性值实际更改时才发出属性更改信号。此外，如果某个属性或一组属性不常使用，则可以为多个属性使用相同的 NOTIFY 信号。这样做时应小心，以确保性能不会受到影响。

NOTIFY 信号的存在确实会带来一些开销。有些情况下，属性的值在对象构造时设置，并且随后不会更改。最常见的情况是当类型使用分组属性时，分组属性对象只分配一次，并且仅在对象删除时释放。在这些情况下，可以在属性声明中添加 CONSTANT 属性，而不是 NOTIFY 信号。

CONSTANT 属性应仅用于在类构造函数中设置并最终确定值的属性。所有其他希望在绑定中使用的属性应具有 NOTIFY 信号。

### 具有Object类型的属性

对象类型的属性可以从 QML 访问，前提是对象类型已在 QML 类型系统中适当注册。

例如，Messge类型可能有一个类型为MessageBody*的body属性：

```C++
class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MessageBody* body READ body WRITE setBody NOTIFY bodyChanged)
public:
    MessageBody* body() const;
    void setBody(MessageBody* body);
};

class MessageBody : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE text NOTIFY textChanged)
// ...
}
```

假设Message类型在QML类型系统中被注册了，就运行其被QML代码所用：

```qml
Message {
    // ...
}
```

如果MessageBody类型也在QML类型系统中注册，那么就可以赋值MessageBody给Message的body属性，这些都在QML代码中完成：

```
Message {
    body: MessageBody {
        text: "Hello, world!"
    }
}
```

### 具有Object-List类型的属性

包含 QObject 派生类型**列表**的属性也可以暴露给 QML。然而，为此目的，应使用 QQmlListProperty 而不是 QList<T> 作为属性类型。这是因为 QList 不是 QObject 派生类型，因此无法通过 Qt 元对象系统提供必要的 QML 属性特性，例如在列表修改时的信号通知。

例如，MessageBoard类有一个类型为QQmlListProperty类型的messages属性，存储Message的所有实例：

```c++
class MessageBoard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Message> messages READ messages)
public:
    QQmlListProperty<Message> messages();

private:
    static void append_message(QQmlListProperty<Message> *list, Message *msg);

    QList<Message *> m_messages;
};
```

MessageBoard::messages() 函数只是从其 QList<T> m_messages 成员创建并返回一个 QQmlListProperty，并传递 QQmlListProperty 构造函数所需的适当列表修改函数：

```c++
QQmlListProperty<Message> MessageBoard::messages()
{
    return QQmlListProperty<Message>(this, 0, &MessageBoard::append_message);
}

void MessageBoard::append_message(QQmlListProperty<Message> *list, Message *msg)
{
    MessageBoard *msgBoard = qobject_cast<MessageBoard *>(list->object);
    if (msg)
        msgBoard->m_messages.append(msg);
}
```

注意，QQmlListProperty 的模板类型（在本例中为 Message）必须在 QML 类型系统中注册。

### 组合的属性

任何只读对象类型的属性都可以作为分组属性从 QML 代码访问。这可以用来暴露一组相关的属性，这些属性描述了一种类型的一组属性。

例如，假设 Message::author 属性的类型是 MessageAuthor 而不是简单的字符串，因为其具有 name 和 email 的子属性：

```c++
class MessageAuthor : public QObject
{
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString email READ email WRITE setEmail)
public:
    ...
};

class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MessageAuthor* author READ author)
public:
    Message(QObject *parent)
        : QObject(parent), m_author(new MessageAuthor(this))
    {
    }
    MessageAuthor *author() const {
        return m_author;
    }
private:
    MessageAuthor *m_author;
};
```

可以使用 QML 中的分组属性语法来写入 author 属性，如下所示：

```
Message {
    author.name: "Alexandra"
    author.email: "alexandra@mail.com"
}
```

作为分组属性暴露的类型与**对象类型**属性不同，分组属性是只读的，并且在构造时由父对象初始化为有效值。分组属性的子属性可以从 QML 修改，但分组属性对象本身永远不会改变，而对象类型属性可以随时从 QML 分配新的对象值。因此，分组属性对象的生命周期严格由 C++ 父实现控制，而对象类型属性可以通过 QML 代码自由创建和销毁。

## 暴露方法（包括Qt槽）

任何 QObject 派生类型的方法都可以从 QML 代码访问，如果它是：

- 用 Q_INVOKABLE() 宏标记的公共方法
- 作为公共 Qt 槽的方法

例如，下面的 MessageBoard 类有一个用 Q_INVOKABLE 宏标记的 postMessage() 方法，以及一个作为公共槽的 refresh() 方法：

```c++
class MessageBoard : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_INVOKABLE bool postMessage(const QString &msg) {
        qDebug() << "Called the C++ method with" << msg;
        return true;
    }

public slots:
    void refresh() {
        qDebug() << "Called the C++ slot";
    }
}
```

如果将 MessageBoard 的实例设置为 MyItem.qml 文件的必需属性，那么 MyItem.qml 可以调用这两个方法，如下例所示：

C++代码：

```c++
int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    MessageBoard msgBoard;
    QQuickView view;
    view.setInitialProperties({{"msgBoard", &msgBoard}});
    view.setSource(QUrl::fromLocalFile("MyItem.qml"));
    view.show();

    return app.exec();
}
```

QML代码：

```c++
// MyItem.qml
import QtQuick 2.0

Item {
    required property MessageBoard msgBoard

    width: 100; height: 100

    MouseArea {
        anchors.fill: parent
        onClicked: {
            var result = msgBoard.postMessage("Hello from QML")
            console.log("Result of postMessage():", result)
            msgBoard.refresh();
        }
    }
}
```

如果 C++ 方法具有 QObject* 类型的参数，则可以使用对象 id 或引用该对象的 JavaScript var 值从 QML 传递参数值。

QML 支持调用重载的 C++ 函数。如果有多个具有相同名称但参数不同的 C++ 函数，将根据提供的参数数量和类型调用正确的函数。

从 C++ 方法返回的值在从 QML 中的 JavaScript 表达式访问时会转换为 JavaScript 值。

### C++方法和this对象

您可能希望从一个对象中检索 C++ 方法并在另一个对象上调用它。请考虑以下示例，其中QML模块名为Example：

C++代码：

```c++
class Invokable : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    Invokable(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void invoke() { qDebug() << "invoked on " << objectName(); }
};
```

QML代码：

```
import QtQml
import Example

Invokable {
    objectName: "parent"
    property Invokable child: Invokable {}
    Component.onCompleted: child.invoke.call(this)
}
```

如果从合适的 main.cpp 加载 QML 代码，它应该打印“invoked on parent”。然而，由于一个长期存在的错误，它并没有这样做。历史上，C++ 方法的 'this' 对象与方法是不可分割地绑定在一起的。改变现有代码的这种行为会导致微妙的错误，因为 'this' 对象在许多地方是隐式的。从 Qt 6.5 开始，您可以显式选择正确的行为，并允许 C++ 方法接受 'this' 对象。为此，请在您的 QML 文档中添加以下 pragma：

```c++
pragma NativeMethodBehavior: AcceptThisObject
```

添加这行代码后，上述示例将按预期工作。

## 暴露信号

任何 QObject 派生类型的公共信号都可以从 QML 代码访问。

QML 引擎会自动为从 QML 使用的任何 QObject 派生类型的信号创建信号处理程序。信号处理程序的命名规则是 on<Signal>，其中 <Signal> 是信号的名称，首字母大写。通过信号传递的所有参数都可以通过参数名称在信号处理程序中使用。

例如，假设 MessageBoard 类有一个带有单个参数 subject 的 newMessagePosted() 信号：

```c++
class MessageBoard : public QObject
{
    Q_OBJECT
public:
   // ...
signals:
   void newMessagePosted(const QString &subject);
};
```

如果 MessageBoard 类型已在 QML 类型系统中注册，那么在 QML 中声明的 MessageBoard 对象可以使用名为 onNewMessagePosted 的信号处理程序接收 newMessagePosted() 信号，并检查 subject 参数值：

```
MessageBoard {
    onNewMessagePosted: (subject)=> console.log("New message received:", subject)
}
```

与属性值和方法参数一样，信号参数必须是 QML 引擎支持的类型；请参阅“QML 和 C++ 之间的数据类型转换”。（使用未注册的类型不会产生错误，但参数值将无法从处理程序访问。）

类可以有多个同名信号，但只有最后一个信号可以作为 QML 信号访问。请注意，具有相同名称但不同参数的信号无法相互区分。



# 在C++中定义QML类型

https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html

在使用 C++ 代码扩展 QML 时，可以将 C++ 类注册到 QML 类型系统中，以便在 QML 代码中将该类用作数据类型。虽然任何 QObject 派生类的属性、方法和信号都可以从 QML 访问，如“暴露 C++ 类型的属性到 QML”中所讨论的，但在注册到类型系统之前，不能将此类用作 QML 中的数据类型。此外，注册还可以提供其他功能，例如允许类作为可实例化的 QML 对象类型从 QML 使用，或允许类的单例实例从 QML 导入和使用。

此外，Qt Qml 模块提供了在 C++ 中实现 QML 特定功能（如附加属性和默认属性）的机制。

（请注意，本文件中涵盖的许多重要概念在“使用 C++ 编写 QML 扩展”教程中进行了演示。）

注意：声明 QML 类型的所有头文件需要从项目的 include 路径中无前缀地访问。

有关 C++ 和不同 QML 集成方法的更多信息，请参阅 **C++ 和 QML 集成概述**页面。

## 在QML类型系统中注册C++类型

可以将 QObject 派生类注册到 QML 类型系统中，以便在 QML 代码中将该类型用作数据类型。

引擎允许注册可实例化和不可实例化的类型。注册可实例化类型使 C++ 类可以用作 QML 对象类型的定义，允许在 QML 代码中使用对象声明来创建此类型的对象。注册还为引擎提供了额外的类型元数据，使该类型（以及类声明的任何枚举）可以用作在 QML 和 C++ 之间交换的属性值、方法参数和返回值、信号参数的数据类型。

注册不可实例化类型也会以这种方式注册类作为数据类型，但该类型不能作为 QML 对象类型从 QML 实例化。这很有用，例如，如果一个类型有应暴露给 QML 的枚举，但该类型本身不应实例化。

### 先决条件

所有下面提到的宏都可以从 qqmlregistration.h 头文件中获得。您需要将以下代码添加到使用这些宏的文件中，以使这些宏可用：

```
#include <QtQml/qqmlregistration.h>
```

此外，您的类声明必须位于通过项目的 include 路径可访问的头文件中。声明用于在编译时生成注册代码，并且注册代码需要包含包含声明的头文件。

### 注册一个可实列化的对象类型

任何 QObject 派生的 C++ 类都可以注册为 QML 对象类型的定义。一旦类在 QML 类型系统中注册，该类就可以像 QML 代码中的任何其他对象类型一样声明和实例化。一旦创建，类实例可以从 QML 操作；正如“暴露 C++ 类型的属性到 QML”所解释的那样，任何 QObject 派生类的属性、方法和信号都可以从 QML 代码访问。

要将 QObject 派生类注册为可实例化的 QML 对象类型，请在类声明中添加 QML_ELEMENT 或 QML_NAMED_ELEMENT(<name>)。您还需要在构建系统中进行调整。对于 qmake，添加 CONFIG += qmltypes、QML_IMPORT_NAME 和 QML_IMPORT_MAJOR_VERSION 到您的项目文件中。对于 CMake，包含类的文件应作为目标设置的一部分，使用 qt_add_qml_module()。这将类注册到给定主要版本的类型命名空间中，使用类名或显式给定的名称作为 QML 类型名称。次要版本将从附加到属性、方法或信号的任何修订中派生。默认次要版本为 0。您可以通过在类声明中添加 QML_ADDED_IN_VERSION() 宏来显式限制类型仅从特定次要版本可用。客户端可以导入命名空间的合适版本以使用该类型。

例如，假设有一个具有 `author` 和 `creationDate` 属性的 `Message` 类：

```c++
class Message : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    Q_PROPERTY(QDateTime creationDate READ creationDate WRITE setCreationDate NOTIFY creationDateChanged)
    QML_ELEMENT
public:
    // ...
};
```

可以通过向项目文件添加适当的类型命名空间和版本号来注册此类型。例如，要使该类型在 com.mycompany.messaging 命名空间中以版本 1.0 可用，可CMake中配置：

```cmake
qt_add_qml_module(messaging
    URI com.mycompany.messaging
    VERSION 1.0
    SOURCES
        message.cpp message.h
)
```

该类型可以在 QML 中的对象声明中使用，并且可以读取和写入其属性，如下例所示：

```
import com.mycompany.messaging

Message {
    author: "Amelie"
    creationDate: new Date()
}
```

### 注册值类型

任何带有 Q_GADGET 宏的类型都可以注册为 QML 值类型。一旦这种类型在 QML 类型系统中注册，它就可以在 QML 代码中用作属性类型。这样的实例可以从 QML 操作；正如“暴露 C++ 类型的属性到 QML”所解释的那样，任何值类型的属性和方法都可以从 QML 代码访问。

与对象类型相比，值类型需要小写名称。注册它们的首选方法是使用 QML_VALUE_TYPE 或 QML_ANONYMOUS 宏。由于您的 C++ 类通常会有大写名称，因此没有与 QML_ELEMENT 等效的宏。否则，注册过程与对象类型的注册非常相似。

例如，假设您想注册一个由两个字符串（first 和 last name）组成的值类型 person：

```
class Person
{
    Q_GADGET
    Q_PROPERTY(QString firstName READ firstName WRITE setFirstName)
    Q_PROPERTY(QString lastName READ lastName WRITE setLastName)
    QML_VALUE_TYPE(person)
public:
    // ...
};
```

值类型有一些进一步的限制：

- 值类型不能是单例。
- 值类型需要是默认可构造和可复制构造的。
- 使用 QProperty 作为值类型的成员是有问题的。值类型会被复制，您需要决定在此时如何处理 QProperty 上的任何绑定。您不应在值类型中使用 QProperty。
- 值类型不能提供附加属性。
- 定义值类型扩展的 API (QML_EXTENDED) 不是公开的，并且可能会在未来发生变化。

### 具有枚举的值类型

将枚举从值类型暴露给 QML 需要一些额外的步骤。

值类型在 QML 中使用小写名称，并且具有小写名称的类型通常在 JavaScript 代码中不可寻址（除非您指定 `pragma ValueTypeBehavior: Addressable`）。如果您在 C++ 中有一个带有枚举的值类型，并希望将其暴露给 QML，则需要单独暴露该枚举。

这可以通过使用 `QML_FOREIGN_NAMESPACE` 解决。首先，从您的值类型派生以创建一个单独的 C++ 类型：

```c++
class Person
{
    Q_GADGET
    Q_PROPERTY(QString firstName READ firstName WRITE setFirstName)
    Q_PROPERTY(QString lastName READ lastName WRITE setLastName)
    QML_VALUE_TYPE(person)
public:
    enum TheEnum { A, B, C };
    Q_ENUM(TheEnum)
    //...
};

class PersonDerived: public Person
{
    Q_GADGET
};
```

然后将该派生类型作为外部名字空间来暴露：

```c++
namespace PersonDerivedForeign
{
    Q_NAMESPACE
    QML_NAMED_ELEMENT(Person)
    QML_FOREIGN_NAMESPACE(PersonDerived)
}
```

这会生成一个名为 Person（大写）的 QML 命名空间，其中包含一个名为 TheEnum 的枚举和值 A、B 和 C。然后，您可以在 QML 中编写如下代码：

```
someProperty: Person.A
```

是的，您仍然可以像以前一样使用名为 person（小写）的值类型。

### 注册不可实列化类型

有时，QObject 派生类可能需要在 QML 类型系统中注册，但不作为可实例化类型。例如，如果一个 C++ 类：

- 是不应实例化的接口类型
- 是不需要暴露给 QML 的基类类型
- 声明了一些应从 QML 访问的枚举，但不应实例化
- 是应通过单例实例提供给 QML 的类型，不应从 QML 实例化



略。。。

## 定义QML特定类型和属性

### 提供附加属性

在 QML 语言语法中，有附加属性和附加信号处理程序的概念，这些是附加到对象的额外属性。本质上，这些属性由附加类型实现和提供，并且这些属性可以附加到另一种类型的对象上。这与普通对象属性不同，普通对象属性由对象类型本身（或对象的继承类型）提供。

例如，下面的Item使用附加的属性和属性handler：

```
import QtQuick 2.0

Item {
    width: 100; height: 100

    focus: true
    Keys.enabled: false
    Keys.onReturnPressed: console.log("Return key was pressed")
}
```

在这里，Item 对象能够访问和设置 Keys.enabled 和 Keys.onReturnPressed 的值。这允许 Item 对象将这些额外的属性作为其自身现有属性的扩展来访问。



### 属性修改器类型

属性修饰符类型是一种特殊的 QML 对象类型。属性修饰符类型实例会影响其应用到的 QML 对象实例的属性。有两种不同类型的属性修饰符类型：

- 属性值写入拦截器
- 属性值来源

### 属性值Source

属性值来源是 QML 类型，可以使用 `<PropertyValueSource> on <property>` 语法随时间自动更新属性的值。例如，QtQuick 模块提供的各种属性动画类型是属性值来源的示例。

可以通过子类化 `QQmlPropertyValueSource` 并提供一个随时间向属性写入不同值的实现来在 C++ 中实现属性值来源。当属性值来源使用 QML 中的 `<PropertyValueSource> on <property>` 语法应用于属性时，引擎会为其提供对该属性的引用，以便更新属性值。

例如，假设有一个 `RandomNumberGenerator` 类要作为属性值来源提供，这样当应用于 QML 属性时，它将每 500 毫秒更新一次属性值为不同的随机数。此外，可以为此随机数生成器提供一个 `maxValue`。该类可以实现如下：

```c++
class RandomNumberGenerator : public QObject, public QQmlPropertyValueSource
{
    Q_OBJECT
    Q_INTERFACES(QQmlPropertyValueSource)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged);
    QML_ELEMENT
public:
    RandomNumberGenerator(QObject *parent)
        : QObject(parent), m_maxValue(100)
    {
        QObject::connect(&m_timer, SIGNAL(timeout()), SLOT(updateProperty()));
        m_timer.start(500);
    }

    int maxValue() const;
    void setMaxValue(int maxValue);

    virtual void setTarget(const QQmlProperty &prop) { m_targetProperty = prop; }

signals:
    void maxValueChanged();

private slots:
    void updateProperty() {
        m_targetProperty.write(QRandomGenerator::global()->bounded(m_maxValue));
    }

private:
    QQmlProperty m_targetProperty;
    QTimer m_timer;
    int m_maxValue;
};
```

当 QML 引擎遇到将 `RandomNumberGenerator` 用作属性值来源时，它会调用 `RandomNumberGenerator::setTarget()` 来提供应用了值来源的属性。当 `RandomNumberGenerator` 的内部计时器每 500 毫秒触发一次时，它会将一个新数字值写入指定的属性。

一旦 `RandomNumberGenerator` 类在 QML 类型系统中注册，它就可以作为属性值来源从 QML 使用。如下所示，它用于每 500 毫秒更改一个 Rectangle 的宽度：

```
import QtQuick 2.0

Item {
    width: 300; height: 300

    Rectangle {
        RandomNumberGenerator on width { maxValue: 300 }

        height: 100
        color: "red"
    }
}
```



# 将C++对象作为上下文属性嵌入到QML

在将 QML 对象加载到 C++ 应用程序中时，直接嵌入一些可以从 QML 代码中使用的 C++ 数据是非常有用的。例如，这使得可以在嵌入的对象上调用 C++ 方法，或使用 C++ 对象实例作为 QML 视图的数据模型。

通过 `QQmlContext` 类可以将 C++ 数据注入到 QML 对象中。该类将数据暴露给 QML 对象的上下文，以便可以直接从 QML 代码的范围内引用这些数据。

## 设置一个简单的上下文属性

例如，这里有一个 QML 项目引用了当前范围内不存在的 `currentDateTime` 值：

```
// MyItem.qml
import QtQuick

Text { text: currentDateTime }
```

这个 `currentDateTime` 值可以通过加载 QML 组件的 C++ 应用程序直接设置，使用 `QQmlContext::setContextProperty()`：

```
QQuickView view;
view.rootContext()->setContextProperty("currentDateTime", QDateTime::currentDateTime());
view.setSource(QUrl::fromLocalFile("MyItem.qml"));
view.show();
```

## 设置一个对象作为上下文属性

上下文属性可以保存 QVariant 或 QObject* 值。这意味着自定义 C++ 对象也可以使用这种方法注入，并且这些对象可以在 QML 中直接修改和读取。这里，我们修改上述示例以嵌入一个 QObject 实例，而不是 QDateTime 值，并且 QML 代码调用对象实例上的方法。

C++代码：

```c++
class ApplicationData : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QDateTime getCurrentDateTime() const {
        return QDateTime::currentDateTime();
    }
};

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQuickView view;

    ApplicationData data;
    view.rootContext()->setContextProperty("applicationData", &data);

    view.setSource(QUrl::fromLocalFile("MyItem.qml"));
    view.show();

    return app.exec();
}
```

QML代码：

```QML
// MyItem.qml
import QtQuick

Text { text: applicationData.getCurrentDateTime() }
```

如果 QML 项目需要从上下文属性接收信号，可以使用 Connections 类型连接到它们。例如，如果 ApplicationData 有一个名为 dataChanged() 的信号，可以在 Connections 对象中使用 onDataChanged 处理程序连接到该信号：

```
Text {
    text: applicationData.getCurrentDateTime()

    Connections {
        target: applicationData
        onDataChanged: console.log("The application data changed!")
    }
}
```

上下文属性在 QML 视图中使用基于 C++ 的数据模型时非常有用。请参见以下示例：

- 字符串 ListModel
- 对象 ListModel
- 抽象项模型 (AbstractItemModel)



# 在C++中与QML对象交互

https://doc.qt.io/qt-6/qtqml-cppintegration-overview.html

所有 QML 对象类型都是 QObject 派生类型，无论它们是由引擎内部实现的还是由第三方定义的。这意味着 QML 引擎可以使用 Qt 元对象系统动态实例化任何 QML 对象类型并检查创建的对象。

这对于从 C++ 代码创建 QML 对象非常有用，无论是显示可以视觉呈现的 QML 对象，还是将非视觉 QML 对象数据集成到 C++ 应用程序中。一旦创建了 QML 对象，就可以从 C++ 检查它，以读取和写入属性、调用方法和接收信号通知。

## 在C++中载入QML对象

QML 文档可以使用 QQmlComponent 或 QQuickView 加载。QQmlComponent 将 QML 文档加载为 C++ 对象，然后可以从 C++ 代码中修改。QQuickView 也可以这样做，但由于 QQuickView 是一个 QWindow 派生类，加载的对象也会被渲染到视觉显示中；QQuickView 通常用于将可显示的 QML 对象集成到应用程序的用户界面中。

例如，假设有一个MyItem.qml文件内容如下：

```
import QtQuick

Item {
    width: 100; height: 100
}
```

这个 QML 文档可以使用 QQmlComponent 或 QQuickView 加载，使用以下 C++ 代码。使用 QQmlComponent 需要调用 QQmlComponent::create() 来创建组件的新实例，而 QQuickView 会自动创建组件的实例，可以通过 QQuickView::rootObject() 访问：

在QQuickView下访问的代码：

```c++
// Using QQuickView
QQuickView view;
view.setSource(QUrl::fromLocalFile("MyItem.qml"));
view.show();
QObject *object = view.rootObject();
```

这个对象是已创建的 MyItem.qml 组件的实例。现在，您可以使用 `QObject::setProperty()` 或 `QQmlProperty::write()` 修改该项目的属性：

```
object->setProperty("width", 500);
QQmlProperty(object, "width").write(500);
```

`QObject::setProperty()` 和 `QQmlProperty::write()` 之间的区别在于，后者在设置属性值的同时还会移除绑定。例如，假设上面的宽度赋值是绑定到高度的：

```
width: height
```

如果在调用 `object->setProperty("width", 500)` 后 Item 的高度发生变化，宽度将再次更新，因为绑定仍然有效。然而，如果在调用 `QQmlProperty(object, "width").write(500)` 后高度发生变化，宽度将不会改变，因为绑定不再存在。

或者，您可以将对象转换为其实际类型并调用方法以确保编译时安全性。在这种情况下，MyItem.qml 的基对象是一个 Item，它由 QQuickItem 类定义。

```c++
QQuickItem *item = qobject_cast<QQuickItem*>(object);
item->setWidth(500);
```

您还可以使用 `QMetaObject::invokeMethod()` 和 `QObject::connect()` 连接到组件中定义的任何信号或调用方法。有关详细信息，请参见下文的“调用 QML 方法和连接到 QML 信号”。

## 通过定义良好的C++接口访问QML对象

与 QML 进行交互的最佳方式是在 C++ 中定义一个接口并在 QML 中访问它。使用其他方法时，重构 QML 代码可能会导致 QML / C++ 交互中断。通过 QML 驱动交互有助于用户和工具（如 qmllint）更容易地推理 QML 和 C++ 代码的交互。从 C++ 访问 QML 会导致 QML 代码在没有手动验证外部 C++ 代码未修改给定 QML 组件的情况下无法理解，即使这样，访问的范围也可能会随时间变化，使得继续使用这种策略成为维护负担。

翻译：要让 QML 驱动交互，首先需要定义一个 C++ 接口：

```c++
class CppInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    // ...
};
```

使用 QML 驱动的方法，可以通过两种方式与该接口进行交互：

### 单例

一种选择是通过在接口中添加 `QML_SINGLETON` 宏将接口注册为单例，向所有组件公开。之后，该接口可以通过一个简单的 import 语句使用。

```C++
import my.company.module

Item {
    Component.onCompleted: {
        CppInterface.foo();
    }
}
```

如果您需要在根组件之外的更多地方使用接口，请使用这种方法，因为简单地传递对象将需要通过属性显式地将其传递给其他组件，或者使用不推荐的慢速方法，即使用不合格的访问。

### 初始属性

另一种选择是通过 `QML_UNCREATABLE` 将接口标记为不可创建，并使用 `QQmlComponent::createWithInitialProperties()` 和 QML 端的必需属性将其提供给根 QML 组件。

你的根组件可能是这样：

```
import QtQuick

Item {
    required property CppInterface interface
    Component.onCompleted: {
        interface.foo();
    }
}
```

将属性标记为必需可以防止在未设置接口属性的情况下创建组件。

然后，您可以使用与“从 C++ 加载 QML 对象”中概述的相同方式初始化组件，只是使用 `createWithInitialProperties()`:

```c++
component.createWithInitialProperties(QVariantMap{{u"interface"_s, QVariant::fromValue<CppInterface *>(new CppInterface)}});
```

如果您知道您的接口只需要在根组件中可用，建议使用这种方法。这也使得在 C++ 端更容易连接到接口的信号和槽。

如果这两种方法都不适合您的需求，您可能需要研究使用 C++ 模型。

## 通过对象名称访问载入的QML对象

QML 组件本质上是具有兄弟姐妹和自己子对象的对象树。可以使用 `QObject::objectName` 属性和 `QObject::findChild()` 定位 QML 组件的子对象。例如，如果 MyItem.qml 的根项目有一个子 Rectangle 项目：

```qml
import QtQuick

Item {
    width: 100; height: 100

    Rectangle {
        anchors.fill: parent
        objectName: "rect"
    }
}
```

可以这样定位子对象：

```c++
QObject *rect = object->findChild<QObject*>("rect");
if (rect)
    rect->setProperty("color", "red");
```

请注意，一个对象可能有多个具有相同 objectName 的子对象。例如，ListView 会创建其委托的多个实例，因此如果其委托声明了一个特定的 objectName，ListView 将有多个具有相同 objectName 的子对象。在这种情况下，可以使用 `QObject::findChildren()` 来查找所有具有匹配 objectName 的子对象。

## 在C++中访问QML对象的成员

### 属性

QML 对象中声明的任何属性都可以从 C++ 自动访问。给定一个像这样的 QML 项目：

```
// MyItem.qml
import QtQuick

Item {
    property int someNumber: 100
}
```

可以使用 `QQmlProperty` 或 `QObject::setProperty()` 和 `QObject::property()` 设置和读取 `someNumber` 属性的值：

```
QQmlEngine engine;
QQmlComponent component(&engine, "MyItem.qml");
QObject *object = component.create();

qDebug() << "Property value:" << QQmlProperty::read(object, "someNumber").toInt();
QQmlProperty::write(object, "someNumber", 5000);

qDebug() << "Property value:" << object->property("someNumber").toInt();
object->setProperty("someNumber", 100);
```

您应该始终使用 `QObject::setProperty()`、`QQmlProperty` 或 `QMetaProperty::write()` 来更改 QML 属性值，以确保 QML 引擎能够意识到属性的变化。例如，假设您有一个自定义类型 PushButton，其 `buttonText` 属性在内部反映了 `m_buttonText` 成员变量的值。直接修改成员变量不是一个好主意。

```c++
//bad code
QQmlComponent component(engine, "MyButton.qml");
PushButton *button = qobject_cast<PushButton*>(component.create());
button->m_buttonText = "Click me";
```

由于值是直接更改的，这绕过了 Qt 的元对象系统，QML 引擎不会意识到属性的更改。这意味着对 `buttonText` 的属性绑定不会更新，任何 `onButtonTextChanged` 处理程序也不会被调用。

### 调用QML方法

所有 QML 方法都暴露在元对象系统中，可以使用 `QMetaObject::invokeMethod()` 从 C++ 调用。您可以在冒号字符后指定参数和返回值的类型，如下面的代码片段所示。这在您希望将 C++ 中具有特定签名的信号连接到 QML 定义的方法时非常有用。如果省略类型，C++ 签名将使用 `QVariant`。

这是一个使用QMetaObject::invokeMethod()来调用QML方法的C++应用：

QML代码：

```
// MyItem.qml
import QtQuick

Item {
    function myQmlFunction(msg: string) : string {
        console.log("Got message:", msg)
        return "some return value"
    }
}
```

C++代码：

```c++
// main.cpp
QQmlEngine engine;
QQmlComponent component(&engine, "MyItem.qml");
QObject *object = component.create();

QString returnedValue;
QString msg = "Hello from C++";
QMetaObject::invokeMethod(object, "myQmlFunction",
        Q_RETURN_ARG(QString, returnedValue),
        Q_ARG(QString, msg));

qDebug() << "QML function returned:" << returnedValue;
delete object;
```

如果在 QML 中省略类型或将其指定为 `var`，那么在调用 `QMetaObject::invokeMethod` 时，必须使用 `Q_RETURN_ARG()` 和 `Q_ARG()` 将类型传递为 `QVariant`。

### 连接到QML信号

所有 QML 信号都可以自动在 C++ 中使用，并且可以像任何普通的 Qt C++ 信号一样使用 `QObject::connect()` 进行连接。相应地，任何 C++ 信号都可以通过信号处理程序被 QML 对象接收。

这是一个具有名为 `qmlSignal` 的信号的 QML 组件，该信号使用字符串类型的参数发出。此信号使用 `QObject::connect()` 连接到 C++ 对象的槽，以便每当发出 `qmlSignal` 时调用 `cppSlot()` 方法：

QML代码：

```
// MyItem.qml
import QtQuick

Item {
    id: item
    width: 100; height: 100

    signal qmlSignal(msg: string)

    MouseArea {
        anchors.fill: parent
        onClicked: item.qmlSignal("Hello from QML")
    }
}
```

C++代码：

```
class MyClass : public QObject
{
    Q_OBJECT
public slots:
    void cppSlot(const QString &msg) {
        qDebug() << "Called the C++ slot with message:" << msg;
    }
};

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQuickView view(QUrl::fromLocalFile("MyItem.qml"));
    QObject *item = view.rootObject();

    MyClass myClass;
    QObject::connect(item, SIGNAL(qmlSignal(QString)),
                     &myClass, SLOT(cppSlot(QString)));

    view.show();
    return app.exec();
}
```



