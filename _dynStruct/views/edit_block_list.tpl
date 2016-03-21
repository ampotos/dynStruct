% include header
<script type="text/javascript" src="/static/js/jquery.dataTables.min.js"></script>
<script type="text/javascript" src="/static/js/dataTables.bootstrap.min.js"></script>
<link rel="stylesheet" href="/static/css/dataTables.bootstrap.min.css"/>

<script type="text/javascript">
  $(document).ready(function() {
  var instance_table = $('#instance_blocks').DataTable( {
  "processing": true,
  "ajax": {
  "url": "struct_instance_get?id_struct={{id_struct}}&instance=true"
  },
  "columnDefs": [
  { "visible": false, "targets": 5 }
  ]
  })

  var potential_table = $('#potential_blocks').DataTable( {
  "processing": true,
  "ajax": {
  "url": "struct_instance_get?id_struct={{id_struct}}&instance=false"
  // remove last element of each row (== block id)
  },
  "columnDefs": [
  { "visible": false, "targets": 5 }
  ]
  })

  instance_table.columns().every( function () {
  var that = this;
  $( 'input', this.footer() ).on( 'keyup change', function () {
  if ( that.search() !== this.value ) {
  that
  .search( this.value )
  .draw();}})})

  potential_table.columns().every( function () {
  var that = this;
  $( 'input', this.footer() ).on( 'keyup change', function () {
  if ( that.search() !== this.value ) {
  that
  .search( this.value )
  .draw();}})})


  $('#instance_blocks').on( 'click', 'tr', function () {
  $(this).toggleClass('active');
  // todo add select
  // remove when non solect
  // a chaque load check if id in select list, active it
  } );

  $('#potential_blocks tbody').on( 'click', 'tr', function () {
  $(this).toggleClass('active');
  // todo add select
  // remove when non select
  // a chaque load check if id in select list, active it
  } );

  //$('#button').click( function () {
  //alert( table.rows('.selected').data().length +' row(s) selected' );
  //} );
});
</script>


<h1 class="text-center"> Actual instances </h1>
<h3 class="text-center"> Select to remove blocks from instances of {{struct_name}} </h3>

<table id="instance_blocks" class="table table-bordered" style="width: 100%">
  <thead>
    <tr>
      <th>start</th>
      <th>size</th>
      <th>malloc caller</th>
      <th>free caller</th>
      <th>detailed view</th>
      <th>id</th>
    </tr>
  </thead>
  <tfoot>
    <tr>
      <th> <input class="form-control" type="text" placeholder="Search adress" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search size" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search malloc caller" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search free caller" /> </th>
      <th>  </th>
      <th>  </th>
    </tr>
  </tfoot>
</table>

<h1 class="text-center"> Potential instances </h1>
<h3 class="text-center"> Select to add blocks as instance of {{struct_name}} </h3>

<table id="potential_blocks" class="table table-bordered" style="width: 100%">
  <thead>
    <tr>
      <th>start</th>
      <th>size</th>
      <th>malloc caller</th>
      <th>free caller</th>
      <th>detailed view</th>
      <th>id</th>
    </tr>
  </thead>
  <tfoot>
    <tr>
      <th> <input class="form-control" type="text" placeholder="Search adress" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search size" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search malloc caller" /> </th>
      <th> <input class="form-control" type="text" placeholder="Search free caller" /> </th>
      <th>  </th>
      <th>  </th>
    </tr>
  </tfoot>
</table>

% include footer
