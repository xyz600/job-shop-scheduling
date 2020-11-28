'use strict';

class ScheduleVisualizer {
    constructor(problem, answer) {
        this.problem = problem;
        this.answer = answer;
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
            title: 'Rectangles Positioned Relative to the Plot and to the Axes',
            xaxis: {
                range: [0, 4],
                showgrid: false
            },
            yaxis: {
                range: [0, 4]
            },
            width: 800,
            height: 600,
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
                    xref: 'paper',
                    yref: 'paper',
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

    show() {
        const layout = this.layout_json();
        const data = this.data_json();
        Plotly.newPlot('myDiv', data, layout);
    }
}

const viewer = new ScheduleVisualizer("123", "456");
viewer.show()