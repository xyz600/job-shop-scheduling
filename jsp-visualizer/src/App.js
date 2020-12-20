import React from 'react';
import Plot from 'react-plotly.js'
import Dropzone from 'react-dropzone'

import './App.css';

class Rectangle {
  constructor(start_time, end_time, machine, selected) {
    this.start_time = start_time;
    this.end_time = end_time;
    this.machine = machine;
    this.selected = selected;
  }
  toJson(config) {
    return {
      type: 'rect',
      xref: 'x',
      yref: 'y',
      x0: config.unit_width * this.start_time,
      x1: config.unit_width * this.end_time,
      y0: config.unit_height * (this.machine - 1),
      y1: config.unit_height * this.machine,
      line: {
        color: (this.selected ? 'rgb(0, 0, 0)' : 'rgb(55, 128, 191)'),
        width: 1
      },
      fillcolor: (this.selected ? 'rgba(0, 0, 0, 0.6)' : 'rgba(55, 128, 191, 0.6)')
    };
  }
}

class App extends React.Component {

  constructor() {
    super()
    this.config = {
      title: "Jobshop Scheduling Problem Visualizer",
      height: 1200,
      width: 1600,
      unit_height: 1,
      unit_width: 1
    };
    this.state = {
      problem: {},
      answer: {},
      data: this.data_json(),
      layout: this.layout_json(),
      debug: true,
      selected_job_id: 0
    };
  }

  is_problem_set = () => {
    return "operations" in this.state.problem;
  }
  is_answer_set = () => {
    return "process_list" in this.state.answer;
  }

  setup_data = (layout, problem, answer, selected_job_id) => {
    layout.shapes.splice(0);
    answer.process_list.forEach((lst, idx) => {
      lst.forEach(elem => {
        const machine_id = idx + 1;
        const duration = problem.operations[elem.operation_id].time;
        const job_id = problem.operations[elem.operation_id].job;
        layout.shapes.push(new Rectangle(
          elem.start_time,
          elem.start_time + duration,
          machine_id,
          selected_job_id == job_id
        ).toJson(this.config));
      });
    });
    let max_time = 0;
    for (let idx in answer.process_list) {
      for (let idx2 in answer.process_list[idx]) {
        max_time = Math.max(max_time, answer.process_list[idx][idx2].start_time);
      }
    }
    layout.xaxis.range = [0, this.config.unit_width * max_time * 1.1];
    layout.yaxis.range = [0, this.config.unit_height * problem.machine_size * 1.1];
  }

  // この記法で関数を定義するのは、bind に関係がある(TODO: 調査)
  onDropProblem = (files) => {
    let reader = new FileReader()
    reader.onloadend = () => {
      const new_problem = JSON.parse(reader.result);
      const cloned_layout = Object.assign(this.state.layout);
      if (this.is_answer_set()) {
        this.setup_data(cloned_layout, new_problem, this.state.answer, this.state.selected_job_id);
      }
      this.setState({
        ...this.state,
        problem: new_problem,
        layout: cloned_layout
      })
    }
    reader.readAsText(files[0]);
  }
  onDropAnswer = (files) => {
    let reader = new FileReader()
    reader.onloadend = () => {
      const new_answer = JSON.parse(reader.result);
      const cloned_layout = Object.assign(this.state.layout);
      if (this.is_problem_set()) {
        this.setup_data(cloned_layout, this.state.problem, new_answer, this.state.selected_job_id);
      }
      this.setState({
        ...this.state,
        answer: new_answer,
        layout: cloned_layout
      })
    }
    reader.readAsText(files[0]);
  }

  onClickDebugCheckBox = () => {
    this.setState({
      ...this.state,
      debug: !this.state.debug
    })
  }
  onChangeJobIDSelect = (event) => {
    const selected_job_id = event.target.value;

    if (this.is_answer_set() && this.is_problem_set()) {
      const cloned_layout = Object.assign(this.state.layout);
      this.setup_data(cloned_layout, this.state.problem, this.state.answer, selected_job_id);

      this.setState({
        ...this.state,
        layout: cloned_layout,
        selected_job_id: selected_job_id
      });
    }
  }

  data_json() {
    return [];
  }

  layout_json() {
    const layout = {
      title: this.config.title,
      width: this.config.width,
      height: this.config.height,
      shapes: [new Rectangle(1, 2, 1).toJson(this.config)],
      xaxis: {
        range: [0, 10]
      },
      yaxis: {
        range: [0, 10]
      }
    };
    return layout;
  }

  debugInfo = () => {
    if (this.state.debug) {
      return (
        <div>
          <div>
            problem = {JSON.stringify(this.state.problem)}
          </div>
          <div>
            answer = {JSON.stringify(this.state.answer)}
          </div>
        </div>
      );
    } else {
      return (
        <div> no debug information. </div>
      );
    }
  }

  render() {
    return (
      <div>
        <Plot
          data={this.state.data}
          layout={this.state.layout}
          config={{ responsive: true }}
        />
        <Dropzone className="dropzone" onDrop={this.onDropProblem}>
          {({ getRootProps, getInputProps }) => (
            <section className="container">
              <div {...getRootProps({ className: 'dropzone' })}>
                <input {...getInputProps()} />
                <p>Drag & Drop problem json</p>
                <p>{this.is_problem_set() ? "problem is set." : ""}</p>
              </div>
            </section>
          )}
        </Dropzone>
        <Dropzone className="dropzone" onDrop={this.onDropAnswer}>
          {({ getRootProps, getInputProps }) => (
            <section className="container">
              <div {...getRootProps({ className: 'dropzone' })}>
                <input {...getInputProps()} />
                <p>Drag & Drop answer json</p>
                <p>{this.is_answer_set() ? "answer is set." : ""}</p>
              </div>
            </section>
          )}
        </Dropzone>
        <label>job_id:</label>
        <input type="text" name="enlighten_job" onChange={this.onChangeJobIDSelect} />
        <form>
          <input type="checkbox" name="debug" value="on" checked={this.state.debug} onChange={this.onClickDebugCheckBox}></input>
          <label> debug mode </label>
        </form>
        {this.debugInfo()}
      </div>
    );
  }
}

export default App;