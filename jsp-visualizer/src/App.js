import React from 'react';
import Plot from 'react-plotly.js'
import Dropzone from 'react-dropzone';

class App extends React.Component {

  constructor() {
    super()
    this.config = {
      title: "Rectangles Positioned Relative to the Plot and to the Axes",
      height: 1200,
      width: 1600
    };
    this.onDropProblem = (files) => {
      let reader = new FileReader()
      reader.onloadend = () => {
        this.setState({
          ...this.state,
          problem: JSON.parse(reader.result)
        })
      }
      reader.readAsText(files[0]);
    };
    this.onDropAnswer = (files) => {
      let reader = new FileReader()
      reader.onloadend = () => {
        this.setState({
          ...this.state,
          answer: JSON.parse(reader.result)
        })
      }
      reader.readAsText(files[0]);
    };
    this.state = {
      problem: {},
      answer: {},
      data: this.data_json(),
      layout: this.layout_json()
    };
  }

  data_json() {
    const trace1 = {
      x: [1.5, 3],
      y: [2.5, 2.5],
      text: ['Rectangle reference to the plot', 'Rectangle reference to the axes'],
      mode: 'text'
    };
    return [trace1];
  }

  layout_json() {
    const layout = {
      title: this.config.title,
      xaxis: {
        range: [0, 4],
        showgrid: false
      },
      yaxis: {
        range: [0, 4]
      },
      width: this.config.width,
      height: this.config.height,
      shapes: [

        //Rectangle reference to the axes

        {
          type: 'rect',
          xref: 'x',
          yref: 'y',
          x0: 2.5,
          y0: 0,
          x1: 3.5,
          y1: 2,
          line: {
            color: 'rgb(55, 128, 191)',
            width: 3
          },
          fillcolor: 'rgba(55, 128, 191, 0.6)'
        },

        //Rectangle reference to the Plot

        {
          type: 'rect',
          xref: 'x',
          yref: 'y',
          x0: 0.25,
          y0: 0,
          x1: 0.5,
          y1: 0.5,
          line: {
            color: 'rgb(50, 171, 96)',
            width: 3
          },
          fillcolor: 'rgba(50, 171, 96, 0.6)'
        }
      ]
    };
    return layout;
  }

  render() {
    return (
      <div>
        <Plot
          data={this.state.data}
          layout={this.state.layout}
        />
        <Dropzone onDrop={this.onDropProblem}>
          {({ getRootProps, getInputProps }) => (
            <section className="container">
              <div {...getRootProps({ className: 'dropzone' })}>
                <input {...getInputProps()} />
                <p>Drag & Drop problem json</p>
              </div>
            </section>
          )}
        </Dropzone>
        <Dropzone onDrop={this.onDropAnswer}>
          {({ getRootProps, getInputProps }) => (
            <section className="container">
              <div {...getRootProps({ className: 'dropzone' })}>
                <input {...getInputProps()} />
                <p>Drag & Drop answer json</p>
              </div>
            </section>
          )}
        </Dropzone>
        <div>
          problem = {JSON.stringify(this.state.problem)}
        </div>
        <div>
          answer = {JSON.stringify(this.state.answer)}
        </div>
      </div>
    );
  }
}

export default App;
