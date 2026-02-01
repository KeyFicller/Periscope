# Periscope

一个用于生成 Mermaid 图表的 C++ 模板库，支持流程图和序列图。

## 快速开始

### 示例 1：流程图

```cpp
#include "graph/periscope_graph.h"
#include "graph/periscope_graph_properties.h"
#include "node/periscope_node.h"
#include "node/periscope_node_properties.h"
#include "link/periscope_link.h"
#include "link/periscope_link_properties.h"
#include "object/periscope_object_properties.h"
#include <iostream>

int main() {
    using namespace periscope;
    
    graph<unsigned int> _graph;
    
    // 创建节点
    node& start = _graph.new_object<node>();
    start.set<OP_name>("开始").set<NP_shape>(NP_shape::k_rectangle);
    
    node& process = _graph.new_object<node>();
    process.set<OP_name>("处理").set<NP_shape>(NP_shape::k_round_rectangle);
    
    node& end = _graph.new_object<node>();
    end.set<OP_name>("结束").set<NP_shape>(NP_shape::k_rectangle);
    
    // 创建连接
    link& link1 = _graph.new_object<link>();
    link1.set<LP_source>(start.get_handle())
         .set<LP_target>(process.get_handle())
         .set<LP_style>(LP_style::k_solid | LP_style::k_arrow_mask);
    
    link& link2 = _graph.new_object<link>();
    link2.set<LP_source>(process.get_handle())
         .set<LP_target>(end.get_handle())
         .set<LP_style>(LP_style::k_solid | LP_style::k_arrow_mask);
    
    // 配置图表
    _graph.template set<GP_type>(graph_type::k_flowchart)
          .template set<GP_output_format>(graph_output_format::k_markdown)
          .template set<GP_flowchart_direction>(
              GP_flowchart_direction<unsigned int>::k_top_to_down);
    
    std::cout << _graph.to_string() << std::endl;
    return 0;
}
```

### 示例 2：序列图

```cpp
#include "graph/periscope_graph.h"
#include "graph/periscope_graph_properties.h"
#include "node/periscope_node.h"
#include "link/periscope_link.h"
#include "object/periscope_object_properties.h"
#include <iostream>

int main() {
    using namespace periscope;
    
    graph<unsigned int> _graph;
    
    // 创建参与者
    node& client = _graph.new_object<node>();
    client.set<OP_name>("Client");
    
    node& server = _graph.new_object<node>();
    server.set<OP_name>("Server");
    
    // 创建交互
    link& req = _graph.new_object<link>();
    req.set<LP_source>(client.get_handle())
       .set<LP_target>(server.get_handle())
       .set<OP_name>("Request");
    
    // 配置图表
    _graph.template set<GP_type>(graph_type::k_sequence)
          .template set<GP_output_format>(graph_output_format::k_markdown)
          .template set<GP_sequence_show_number>(true);
    
    std::cout << _graph.to_string() << std::endl;
    return 0;
}
```

## API 概览

### 创建对象

```cpp
// 创建节点
node& n = graph.new_object<node>();

// 创建连接
link& l = graph.new_object<link>();
```

### 设置属性

```cpp
// 节点属性
node.set<OP_name>("名称")                    // 设置名称
    .set<NP_shape>(NP_shape::k_rectangle);   // 设置形状

// 连接属性
link.set<LP_source>(source.get_handle())     // 设置源节点
    .set<LP_target>(target.get_handle())     // 设置目标节点
    .set<LP_style>(LP_style::k_solid | LP_style::k_arrow_mask)  // 设置样式
    .set<OP_name>("标签");                   // 设置标签

// 图表属性
graph.template set<GP_type>(graph_type::k_flowchart)  // 设置图表类型
      .template set<GP_output_format>(graph_output_format::k_markdown)  // 输出格式
      .template set<GP_flowchart_direction>(GP_flowchart_direction<unsigned int>::k_top_to_down)  // 流程图方向
      .template set<GP_sequence_show_number>(true);  // 序列图显示编号
```

## 属性说明

### 节点形状 (NP_shape)
- `k_rectangle` - 矩形
- `k_round_rectangle` - 圆角矩形
- `k_diamond` - 菱形
- `k_parallelogram` - 平行四边形

### 连接样式 (LP_style)
- `k_solid` - 实线
- `k_dashed` - 虚线
- `k_bold_solid` - 粗实线
- `k_arrow_mask` - 箭头（可与其他样式组合使用 `|`）

### 流程图方向 (GP_flowchart_direction)
- `k_top_to_down` - 从上到下 (TD)
- `k_down_to_top` - 从下到上 (BT)
- `k_left_to_right` - 从左到右 (LR)
- `k_right_to_left` - 从右到左 (RL)

### 图表类型 (GP_type)
- `k_flowchart` - 流程图
- `k_sequence` - 序列图

### 输出格式 (GP_output_format)
- `k_markdown` - Markdown 格式（带代码块）
- `k_mermaid` - 纯 Mermaid 格式

## 特性

- **类型安全**：使用模板元编程确保类型安全
- **属性系统**：灵活的属性系统，支持链式调用
- **多种图表**：支持流程图和序列图
- **自动格式化**：自动生成 Mermaid 代码
